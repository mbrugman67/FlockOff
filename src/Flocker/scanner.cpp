#include <map>
#include <string>
#include <list>

#include <WiFi.h>
#include <NimBLEDevice.h>
#include <NimBLEScan.h>
#include <NimBLEAdvertisedDevice.h>
#include <MD5Builder.h>
#include <ArduinoJson.h>

#include "cli.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"

#include "scanner.h"
#include "flockLog.h"
#include "flockCfg.h"

#include "globals.h"
#include "alloc.h"

#define BIG_BUF_SIZE (2 * 1024 * 1024)

#define SCAN_HOP_TIME_MS 50
#define BT_INTERVAL_MS 1000

// There are 16 subtypes of data frames
// Note that the "CF" data frames have been defined,
// but I don't know if there are any actual implementations
// of them - if you see one, it may be some custom thing
// hijacking the subtype field
#define DATA_FRAME_SUBTYPE_DATA 0x00
#define DATA_FRAME_SUBTYPE_DATA_CF_ACK 0x01
#define DATA_FRAME_SUBTYPE_DATA_CF_POLL 0x02
#define DATA_FRAME_SUBTYPE_DATA_CF_ACK_CF_POLL 0x03
#define DATA_FRAME_SUBTYPE_DATA_NO_DATA_NULL 0x04
#define DATA_FRAME_SUBTYPE_DATA_NO_DATA_CF_ACK 0x05
#define DATA_FRAME_SUBTYPE_DATA_NO_DATA_POLL 0x06
#define DATA_FRAME_SUBTYPE_DATA_NO_DATA_CF_ACK_CF_POLL 0x07
#define DATA_FRAME_SUBTYPE_DATA_QOS_DATA 0x08
#define DATA_FRAME_SUBTYPE_DATA_QOS_CF_ACK 0x09
#define DATA_FRAME_SUBTYPE_DATA_QOS_CF_POLL 0x0a
#define DATA_FRAME_SUBTYPE_DATA_QOS_CF_ACK_CF_POLL 0x0b
#define DATA_FRAME_SUBTYPE_DATA_QOS_NO_DATA 0x0c
#define DATA_FRAME_SUBTYPE_DATA_RESERVED_1 0x0d
#define DATA_FRAME_SUBTYPE_DATA_NO_DATA_QOS_POLL 0x0e
#define DATA_FRAME_SUBTYPE_DATA_NO_DATA_QOS_CF_ACK_CF_POLL 0x0f

// There are 16
// subtypes of management frames (as #defined here).  The packet
// structure is different for each subtype, but in general:
//   HEADER (fixed size)
//   FIXED DATA (variable number of bytes, zero or more)
//   TAGGED DATA (variable number of tagged parameters, zero or more)
//   CRC32 (4 bytes)
#define MGMT_FRAME_SUBTYPE_ASSOCIATION_REQUEST 0x00
#define MGMT_FRAME_SUBTYPE_ASSOCIATION_RESPONSE 0x01
#define MGMT_FRAME_SUBTYPE_REASSOCIATION_REQUEST 0x02
#define MGMT_FRAME_SUBTYPE_REASSOCIATION_RESPONSE 0x03
#define MGMT_FRAME_SUBTYPE_PROBE_REQUEST 0x04
#define MGMT_FRAME_SUBTYPE_PROBE_RESPONSE 0x05
#define MGMT_FRAME_SUBTYPE_TIMING_ADVERTISEMENT 0x06
#define MGMT_FRAME_SUBTYPE_RESERVED_1 0x07
#define MGMT_FRAME_SUBTYPE_BEACON 0x08
#define MGMT_FRAME_SUBTYPE_ATIM 0x09
#define MGMT_FRAME_SUBTYPE_DISASSOCIATION 0x0a
#define MGMT_FRAME_SUBTYPE_AUTHENTICATION 0x0b
#define MGMT_FRAME_SUBTYPE_DEAUTHENTICATION 0x0c
#define MGMT_FRAME_SUBTYPE_ACTION 0x0d
#define MGMT_FRAME_SUBTYPE_ACTION_NO_ACK 0x0e
#define MGMT_FRAME_SUBTYPE_RESERVED2 0x0f

// Each of the management frame subtypes (defined above) has zero or
// more fixed bytes of parameter data before getting to the zero or
// more tagged parameters.  This array holds the number of bytes
// for each subtype
uint8_t fixedParameterLength[] = {
  4,  // 0x00 - length of fixed parameters in assoc request subtype
  6,  // 0x01 - length of fixed parameters in assoc response subtype
  10, // 0x02 - length of fixed parameters in reassoc request subtype
  6,  // 0x03 - length of fixed parameters in reassoc response subtype
  0,  // 0x04 - length of fixed parameters in probe request subtype
  12, // 0x05 - length of fixed parameters in probe response subtype
  0,  // 0x06 - timing advertisement
  0,  // 0x07 - reserved subtype
  12, // 0x08 - length of fixed parameters in beacon subtype
  0,  // 0x09 - ATIM (announcement traffic indication message - something else entirely)
  2,  // 0x0a - length of fixed parameters in dissociation subtype
  6,  // 0x0b - length of fixed parameters in authentication subtype
  2,  // 0x0c - length of fixed parameters in deauthentication subtype
  17, // 0x0d - length of fixed parameters in action subtype
  0,  // 0x0e - length of fixed parameters in action no-ack subtype
  0   // 0x07 - reserved subtype
};

// variable length tagged parameter in a management packet.  A tagged parameter
// is made up of the ID, length of the data, and the data itself.  For
// 802.11 packets, parameter 0 is the SSID, up to 32 bytes long
struct __attribute__((packed)) wifi_ieee80211_mgmt_tagged_parameters_t
{
  uint8_t parameter_id; // parameter ID
  uint8_t length;       // length of parameter
  uint8_t data[0];      // parameter data
};

// 802.11 header
struct __attribute__((packed)) wifi_ieee80211_mac_hdr_t
{
  uint16_t frame_ctrl;
  uint16_t duration_id;
  uint8_t addr1[6]; // receiver address
  uint8_t addr2[6]; // sender address
  uint8_t addr3[6]; // filtering address
  uint16_t sequence_ctrl;
  uint8_t frame[0]; // frame data - start of fixed data (if any) and then tagged parameters (if any)
};

// WiFi channels
static const uint8_t channels[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
                                   36, 40, 44, 48, 149, 153, 157, 161, 165};

static int16_t minBTRSSI;
static int16_t minWiFiRSSI;
const size_t channelCount = sizeof(channels) / sizeof(channels[0]);
static uint16_t channelInx = 0;
static bool surveying = false;
static bool continuousScanning = false;
static MD5Builder hasher;
static NimBLEScan* btScanner = nullptr;
static uint8_t md5sum[16];

static std::map<uint32_t, found_wifi_t, std::less<>, psramAlloc<std::map<uint32_t, found_wifi_t>::value_type>> wifiDevices;
static std::map<uint32_t, found_ble_t, std::less<>, psramAlloc<std::map<uint32_t, found_ble_t>::value_type>> bleDevices;

static std::map<uint32_t, found_wifi_t>::const_iterator citWifiDevices;
static std::map<uint32_t, found_ble_t>::const_iterator citBleDevices;

void reverseBytes(uint8_t* data, size_t len);
const char* wifiPktTypeToText(enum wifi_pkt_t t);
const char* WiFiMgmtSubtypeToText(uint8_t stype);
const char* WiFiDataSubtypeToText(uint8_t stype);

static FLOGGER scanLog;
static bool loggerOK = false;

/****************************************************
* wiFiMatch()
*****************************************************
* Returns match type on WiFi device match
*****************************************************/
wifi_match_t wiFiMatch(const found_wifi_t& w, utils::string& info)
{
  return (scanTargets.isWiFiMatch(w, info));
}

/*************************************************
* Promiscuous mode packet callback handler
*
**************************************************
* If an 802.11 packet of the WIFI_MANAGEMENT or
* WIFI_DATA is seen, add it to a std::map of results.
*
* Note, before adding, check to see if that one
* is already in the map to prevent dups
*
* Frankly, it blows my mind that a tiny six buck
* microcontroller has the resources to handle
* STL containers (with iterators, no less!).
*************************************************/
void wifi_pkt_hndlr(void* buff, wifi_promiscuous_pkt_type_t type)
{
  bool goodPkt = false;

  // make some sense of the buffer.  The passed buffer is of type promiscuous packet;
  // we dont' care about most of that (other than the packet length), so just skip
  // right to the payload, which is the start of the header
  const wifi_promiscuous_pkt_t* ppkt = (wifi_promiscuous_pkt_t*)buff;
  const wifi_ieee80211_mac_hdr_t *hdr = (wifi_ieee80211_mac_hdr_t *)ppkt->payload;

  // if the signal is below minimum RSSI, just bail now
  if (ppkt->rx_ctrl.rssi < flockCfg.getMinRSSI())
  {
    return;
  }

  // a place to store the results
  found_wifi_t wifi;
  memset(wifi.ssid, 0, SSID_LEN + 1);   // clear buffer for ssid

  // keep track of the frame subtype
  wifi.subtype = ((hdr->frame_ctrl & 0xFF) >> 4) & 0x0F;

  // get the length of the packet (minus header stuffs and CRC32);
  // Do a quick sanity check to be sure the length is at least the
  // size of the header + CRC; if not, bail
  if (ppkt->rx_ctrl.sig_len < (sizeof(*hdr) + 4))
  {
      return;
  }

  size_t pktLen = ppkt->rx_ctrl.sig_len - sizeof(*hdr) - 4; // last 4 is the CRC32

  // this is a management packet - a beacon, probe request/response,
  if (type == WIFI_PKT_MGMT)
  {
    wifi.type = wifi_management;

    // remaining packet length, reduced by count of fixed data bytes
    if (pktLen > fixedParameterLength[wifi.subtype])
    {
      pktLen -= fixedParameterLength[wifi.subtype];
    }
    else
    {
      return; // this is one of the short management packets without any tagged data
    }

    // make sense out of the payload.  This depends a lot on the frame subtype; each subtype
    // has zero or more bytes if "fixed data", followed by zero or more tagged parameters.
    // skip past the fixed data bytes to get to the first tagged parameter
    const wifi_ieee80211_mgmt_tagged_parameters_t* taggedPar =
          (wifi_ieee80211_mgmt_tagged_parameters_t*)&hdr->frame[fixedParameterLength[wifi.subtype]];

    // try to get SSID tagged parameter
    if (pktLen > sizeof(wifi_ieee80211_mgmt_tagged_parameters_t))
    {
      // is this parameter an SSID?
      if (taggedPar->parameter_id == 0)
      {
        // parameter length is the length of the SSID char array.
        // NOT ZERO TERMED, max 32 chars
        if (taggedPar->length && (taggedPar->length < SSID_LEN))
        {
          memcpy(wifi.ssid, taggedPar->data, taggedPar->length);
        }
        else
        {
          // does someone feel smart?  :/
          strncpy(wifi.ssid, "<HIDDEN>", SSID_LEN);
        }
      } // the first tagged parameter is the SSID
    goodPkt = true;
    } // there is at least one tagged parameter
  } // this is a management packet
  else if (type == WIFI_PKT_DATA)
  {
    goodPkt = true;

    wifi.type = wifi_data;
  } // this is a data packet

  if (goodPkt)
  {
    // populate the rest of the the results
    wifi.channel = channels[channelInx];      // wifi channel
    wifi.rssi = ppkt->rx_ctrl.rssi;           // signal strength
    memcpy(wifi.sourceAddr, hdr->addr2, 6);   //
    memcpy(wifi.destAddr, hdr->addr1, 6);     //
    wifi.timestamp = millis();                // system timestamp (used for aging)

    utils::string matchInfo;
    wifi_match_t match = wiFiMatch(wifi, matchInfo);

    if (surveying || match != WIFI_MATCH_NONE)
    {
      utils::string seed;
      seed = macToText(wifi.sourceAddr);
      seed += wifiPktTypeToText(wifi.type);
      seed += wifi.ssid;

      hasher.begin();
      hasher.add((uint8_t*)seed.c_str(), seed.size());
      hasher.calculate();
      hasher.getBytes(md5sum);

      // kinda hokey, the key are the first 4 bytes of the MD5 hash of the struct.  Shouldn't be collisions....
      uint32_t key = ((uint32_t)md5sum[0] << 24) | ((uint32_t)md5sum[1] << 16) |
                      ((uint32_t)md5sum[2] << 8) | ((uint32_t)md5sum[3]);

      // have we seen this one already?
      citWifiDevices = wifiDevices.find(key);
      if (citWifiDevices == wifiDevices.end())
      {
        // no?  then add it
        wifiDevices[key] = wifi;

        if (!surveying && loggerOK && flockCfg.getScanLogEnabledState())
        {
          switch (match)
          {
            case WIFI_MATCH_MAC:
            {
              Serial.printf(CLI_BOLD_RED "ALERT!" CLI_RESET CLI_YEL " %s Matched MAC %s (%s)\r\n" CLI_RESET, 
                  gps.getTimeLocationString(), macToText(wifi.sourceAddr), matchInfo.c_str());
              scanLog.addLogLine("WIFI", "%s; Matched mac %s\r\n", gps.getTimeLocationString(), macToText(wifi.sourceAddr)); 
            }  break;

            case WIFI_MATCH_SSID: 
            {
              Serial.printf(CLI_BOLD_RED "ALERT!" CLI_RESET CLI_YEL " %s Matched SSID %s (%s)\r\n" CLI_RESET, 
                  gps.getTimeLocationString(), wifi.ssid, matchInfo.c_str());
              scanLog.addLogLine("WIFI", "%s, Matched on ssid %s\r\n", gps.getTimeLocationString(), wifi.ssid); 
            }  break;
          }
        }
      }
    } // packet is a match! (or we are doing a survey)
  } // adding good packet to map
} // promisuous wifi packet handler


class btAdvertisedCBs : public NimBLEScanCallbacks
{
  void onResult(const NimBLEAdvertisedDevice* advertisedDevice)
  {
    // if signal is below minimum RSSI, bail now
    if (advertisedDevice->getRSSI() < flockCfg.getMinRSSI())
    {
      return;
    }

    resetDeviceHolder();
    btDevice.timestamp = millis();
    btDevice.rssi = advertisedDevice->getRSSI();

    //const uint8_t* macVal = advertisedDevice->getAddress().getVal();
    memcpy(btDevice.mac, advertisedDevice->getAddress().getVal(), 6);
    reverseBytes(btDevice.mac, 6);

    if (advertisedDevice->haveName())
    {
      std::string nimbleName = advertisedDevice->getName();
      size_t nameInx = 0;
      if (nimbleName.length() > 0)
      {
        memset(btDevice.name, 0, SSID_LEN + 1);

        for (size_t ii = 0; ii < nimbleName.length() && ii < 31; ++ii)
        {
          uint8_t c = (uint8_t)nimbleName[ii];
          if (isprint(c))
          {
            btDevice.name[nameInx++] = (char)c;
          }
        }

        // all non-printable characters in BLE device name?
        if (!strlen(btDevice.name))
        {
            strncpy(btDevice.name, "<UNKNOWN>", SSID_LEN);
        }
      }
      else
      {
        strncpy(btDevice.name, "<UNKNOWN>", SSID_LEN);
      }
    }

    if (advertisedDevice->haveServiceUUID())
    {
      // just get the first service UUID
      BLEUUID serviceUUID = advertisedDevice->getServiceUUID(0);
      const uint8_t* svcid = serviceUUID.getValue();

      if (serviceUUID.bitSize() == 16)
      {
        uint16_t uuid16 = (((uint16_t)svcid[1] << 8) & 0xff00) | ((uint16_t)svcid[0] & 0x00ff);
        btDevice.services16 = uuid16;
      }
      else if (serviceUUID.bitSize() == 128)
      {
        btDevice.services128 = serviceUUID.toString();
      }
      else
      {
        Serial.printf(CLI_RED "Unknown service size of %d bits\r\n" CLI_RESET, serviceUUID.bitSize());
      }
    }

    if (advertisedDevice->haveServiceData())
    {
      // just get the first one
      BLEUUID serviceDataUUID = advertisedDevice->getServiceDataUUID(0);
      const uint8_t* svcData = serviceDataUUID.getValue();

      if (serviceDataUUID.bitSize() == 16)
      {
        uint16_t uuid16 = (((uint16_t)svcData[1] << 8) & 0xff00) | ((uint16_t)svcData[0] & 0x00ff);
        btDevice.serviceData16 = uuid16;
      }
      else if (serviceDataUUID.bitSize() == 128)
      {
        btDevice.services128 = serviceDataUUID.toString();
      }
      else
      {
        Serial.printf(CLI_RED "Unknown service data size of %d bits\r\n" CLI_RESET, serviceDataUUID.bitSize());
      }
    }

    hasher.begin();
    hasher.add(btDevice.mac, 6);
    hasher.calculate();
    hasher.getBytes(md5sum);

    // kinda hokey, the key are the first 4 bytes of the MD5 hash of the struct.  Shouldn't be collisions....
    uint32_t key = ((uint32_t)md5sum[0] << 24) | ((uint32_t)md5sum[1] << 16) |
                    ((uint32_t)md5sum[2] << 8) | ((uint32_t)md5sum[3]);    

    // have we seen this one already?
    citBleDevices = bleDevices.find(key);
    if (citBleDevices == bleDevices.end())
    {
      // no?  then add it
      bleDevices[key] = btDevice;
    }
  }

private:
  found_ble_t btDevice;
  std::vector<uint8_t> payload;
  std::vector<uint8_t>::const_iterator citPayload;

  void resetDeviceHolder()
  {
    memset(btDevice.name, 0, SSID_LEN + 1);
    memset(btDevice.mac, 0, 6);
    btDevice.services16 = 0;
    btDevice.services128.clear();
    btDevice.serviceData16 = 0;
    btDevice.serviceData128.clear();
  }
};


/************************************************
* begin()
*************************************************
* Scanner class constructor
*************************************************/
bool SCANNER::begin()
{
  channelInx = 0;
  surveying = false;
  scannerRunning = false;

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // turn off promiscuity for now
  esp_wifi_set_promiscuous(false);

  // we're only interested in MANAGEMENT and DATA frames
	const wifi_promiscuous_filter_t filt = {.filter_mask = (WIFI_PROMIS_FILTER_MASK_MGMT | WIFI_PROMIS_FILTER_MASK_DATA)};
	esp_wifi_set_promiscuous_filter(&filt);

  // set callback
  esp_wifi_set_promiscuous_rx_cb(&wifi_pkt_hndlr);

  // start with first channel
  esp_wifi_set_channel(channels[channelInx], WIFI_SECOND_CHAN_NONE);

  return (true);
}

// Static pointers used for allocation of BT callback
// class.
static btAdvertisedCBs* btCBs = nullptr;
static void* btCBMem = nullptr;

/************************************************
* startBLE()
*************************************************
* Configure and start the Bluetooth scanner
*************************************************/
void SCANNER::startBLE()
{
  if (btScanner)
  {
    this->stopBLE();
    delay(50);
  }

  // doing a placement new to allocate the btAdvertisedCBs class
  // this may seem goofy, but we want to allocate in PSRAM instead
  // of on-die SRAM
  btCBMem = ps_malloc(sizeof(btAdvertisedCBs));
  btAdvertisedCBs* btCBs = new (btCBMem) btAdvertisedCBs();

  BLEDevice::init("");
  btScanner = BLEDevice::getScan();
  btScanner->setScanCallbacks(btCBs, true);
  btScanner->setActiveScan(true);
  btScanner->setInterval(100);
  btScanner->setWindow(99);
  btScanner->setDuplicateFilter(false);
  btScanner->start(0, false);
}

/************************************************
* stopBLE()
*************************************************
* Stop the Bluetooth scanner and release resources
*************************************************/
void SCANNER::stopBLE()
{
  if (btScanner)
  {
    btScanner->stop();
    delay(200);
    btScanner->clearResults();
    NimBLEDevice::deinit(true);
    btScanner = nullptr;

    // because we did placement new, we need to explicitly call the destructor
    if (btCBs != nullptr)   btCBs->~btAdvertisedCBs();
    if (btCBMem != nullptr) free (btCBMem);
    btCBs = nullptr;
    btCBMem = nullptr;
  }
}

/************************************************
* update()
*************************************************
* Call this method periodically
*************************************************/
void SCANNER::update()
{
    if (scannerRunning)
    {
        if ((millis() - btTimeOffset) > BT_INTERVAL_MS)
        {
            btTimeOffset = millis();

            startBLE();
            busyDelayLoop(100);
            stopBLE();
        }

        // handle WiFi channel switching
        if ((millis() - channelTimeOffset) > channelTime)
        {
            channelTimeOffset = millis();
            ++channelInx;
            channelInx %= channelCount;
            esp_wifi_set_channel(channels[channelInx], WIFI_SECOND_CHAN_NONE);
        }

        // things to do when continuously scanning
        if (continuousScanning)
        {
            if (Serial.available())
            {
                stopScanning();
                Serial.read();  // read the character that stopped the scan
            }
            else
            {
              // remove found devices when they timeout
              for (citWifiDevices = wifiDevices.begin(); citWifiDevices != wifiDevices.end(); ++citWifiDevices)
              {
                if (millis() > (citWifiDevices->second.timestamp + (flockCfg.getScanHoldTime() * 1000)))
                {
                  if (loggerOK && flockCfg.getScanLogEnabledState())
                  {
                    Serial.printf(CLI_CYA "%s; Removed timed-out WiFi MAC %s (SSID %s)\r\n" CLI_RESET, 
                        gps.getTimeLocationString(), macToText(citWifiDevices->second.sourceAddr), citWifiDevices->second.ssid);
                    scanLog.addLogLine("WIFI", "Removed timedout mac %s\r\n", macToText(citWifiDevices->second.sourceAddr));
                  }
                  wifiDevices.erase(citWifiDevices->first);
                }

                if (wifiDevices.size())
                {
                  flockLED.cycleRed(LEDS::LED_COMMS, 1000, 666);
                }
                else
                {
                  flockLED.stopRed(LEDS::LED_COMMS);
                }
              }
            }
        } // doing continuous scan
        else if (surveying)
        {
            if (!surveyStopTrigger)
            {
                if (channelInx)
                {
                    surveyStopTrigger = true;
                }
            }
            else if (!channelInx)
            {
                Serial.printf(CLI_RESET "\r\n\r\n");
                stopScanning();
                postSurveyActivities();
            }
        }
        else
        {
            // we shouldn't be here, shut it down
            Serial.printf(CLI_BOLD_RED "\r\nStopping un-started scan!\r\n" CLI_RESET);
            stopScanning();
        }
    }
}

/************************************************
* stopScanning()
*************************************************
* Shutdown any scan (continuous or survey)
*************************************************/
void SCANNER::stopScanning()
{
    continuousScanning = false;
    scannerRunning = false;
    surveying = false;

    holdCLI(false);
    esp_wifi_set_promiscuous(false);
    stopBLE();

    flockLED.stopRed(LEDS::LED_COMMS);
    flockLED.stopGrn(LEDS::LED_COMMS);
    flockLED.stopBlu(LEDS::LED_COMMS);

    if (loggerOK)
    {
        scanLog.addLogLine("SCAN", "Ending scan, closing log.\r\n");
        scanLog.flushNow();
        scanLog.close();
        loggerOK = false;
    }

    flockLog.addLogLine("SCAN", "Stopping scanner.\r\n");
    Serial.printf(CLI_RESET "\r\n");
}

/************************************************
* startWiFi()
*************************************************
* Centralized method to turn on promicuous mode
*************************************************/
void SCANNER::startWiFi()
{
    scannerRunning = true;
    holdCLI(true);
    channelInx = 0;

    bleDevices.clear();
    wifiDevices.clear();
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(channels[channelInx], WIFI_SECOND_CHAN_NONE);
}

/************************************************
* scan()
*************************************************
* Begin the continuous scan process
*************************************************/
void SCANNER::scan(const char *logname)
{
    if (logname && strlen(logname))
    {
        loggerOK = scanLog.begin(500, logname, flockCfg.getScanLogFileCount());
    }
    else
    {
        loggerOK = scanLog.begin(500, "autoscan", flockCfg.getScanLogFileCount());
    }

    channelTime = SCAN_HOP_TIME_MS;
    channelTimeOffset = btTimeOffset = millis();

    startWiFi();

    continuousScanning = true;
    flockLED.steadyBlu(LEDS::LED_COMMS, flockCfg.getLEDBrightness());

    Serial.printf(CLI_BOLD_CYA "Starting scan - press any key to stop\r\n" CLI_RESET);
    flockLog.addLogLine("SCAN", "scan() starting\r\n");
}

/************************************************
* survey()
*************************************************
* Begin a single scan process
*************************************************/
void SCANNER::survey(uint32_t interval, const char* fname, const char* notes)
{
    if (fname)      strncpy(surveyFileName, fname, 119);
    else            memset(surveyFileName, '\0', 120);

    if (notes)      strncpy(surveyNotes, notes, 119);
    else            memset(surveyNotes, '\0', 120);

    channelTimeOffset = btTimeOffset = millis();
    channelTime = interval;
    surveying = true;
    surveyStopTrigger = false;

    startWiFi();

    flockLED.alertBlu(LEDS::LED_COMMS);
    flockLED.alertGrn(LEDS::LED_COMMS);

    Serial.printf(CLI_CYA "Survey starting\r\n" CLI_RESET);
    flockLog.addLogLine("SCAN", "survey() starting\r\n");
}

/************************************************
* postSurveyActivities()
*************************************************
* Process the data after a single survey scan
*************************************************/
void SCANNER::postSurveyActivities()
{
    JsonDocument sur;
    if (strlen(surveyNotes))      sur["SurveyNotes"] = surveyNotes;
    else                          sur["SurveyNotes"] = "Generic survey";

    sur["Device"] = flockCfg.getDeviceName();

    JsonArray location = sur["LocationLongLat"].to<JsonArray>();
    location.add(gps.getLongitude());
    location.add(gps.getLatitude());
    sur["SatelliteCount"] = gps.getSatelliteCount();

    time_t t = time(NULL);
    tm *tmp;
    tmp = localtime(&t);
    char tstring[64];
    strftime(tstring, 63, "%F %T", tmp);
    sur["DateTime"] = tstring;
    sur["Timezone"] = flockCfg.getTimeZone();
    sur["DataVersion"] = SURVEY_JSON_VERSION;

    JsonArray devs = sur["WiFiDevices"].to<JsonArray>();
    JsonDocument dev;

    for (citWifiDevices = wifiDevices.begin(); citWifiDevices != wifiDevices.end(); ++citWifiDevices)
    {
        dev.clear();
        dev["Method"] = discoveryToText(WIFI_DISCOVERY);
        dev["Type"] = wifiPktTypeToText(citWifiDevices->second.type);
        if (citWifiDevices->second.type == wifi_management)
        {
            dev["Subtype"] = WiFiMgmtSubtypeToText(citWifiDevices->second.subtype);
        }
        else
        {
            dev["Subtype"] = WiFiDataSubtypeToText(citWifiDevices->second.subtype);
        }
        dev["SSID"] = citWifiDevices->second.ssid;
        dev["SourceAddr"] = macToText(citWifiDevices->second.sourceAddr);
        dev["DestAddr"] = macToText(citWifiDevices->second.destAddr);
        dev["Channel"] = citWifiDevices->second.channel;
        dev["RSSI"] = citWifiDevices->second.rssi;

        devs.add(dev);
    } // adding WiFi devices

    JsonArray btdevs = sur["BLEDevices"].to<JsonArray>();

    for (citBleDevices = bleDevices.begin(); citBleDevices != bleDevices.end(); ++citBleDevices)
    {
        dev.clear();
        dev["Method"] = discoveryToText(BTLE_DISCOVERY);
        dev["Name"] = citBleDevices->second.name;
        dev["MAC"] = macToText(citBleDevices->second.mac);
        dev["RSSI"] = citBleDevices->second.rssi;

        JsonArray uid16 = dev["UUID16bit"].to<JsonArray>();
        if (citBleDevices->second.services16)
        {
          uid16.add(citBleDevices->second.services16);
        }

        JsonArray uid128 = dev["UUID128bit"].to<JsonArray>();
        if (citBleDevices->second.services128.size())
        {
          uid128.add(citBleDevices->second.services128.c_str());
        }

        JsonArray duid16 = dev["DataUUID16bit"].to<JsonArray>();
        if (citBleDevices->second.serviceData16)
        {
          duid16.add(citBleDevices->second.serviceData16);
        }

        JsonArray duid128 = dev["DataUUID128bit"].to<JsonArray>();
        if (citBleDevices->second.serviceData128.size())
        {
          duid128.add(citBleDevices->second.serviceData128.c_str());
        }

        btdevs.add(dev);
    } // adding Bluetooth items to JSON

    Serial.printf(CLI_CYA "Survey done, found " CLI_GRN "%d" CLI_CYA " WiFi devices, " CLI_GRN "%d" CLI_CYA " Bluetooth devices.\r\n" CLI_RESET,
            wifiDevices.size(), bleDevices.size());

    if (!strlen(surveyFileName))
    {
        serializeJsonPretty(sur, Serial);
        Serial.printf("\r\n");
    }
    else
    {
        Serial.printf(CLI_CYA "Saving survey results to " CLI_GRN "%s\r\n" CLI_RESET, surveyFileName);

        char* output = (char*)ps_malloc(BIG_BUF_SIZE + 1);
        memset(output, 0, BIG_BUF_SIZE + 1);

        if (output)
        {
            serializeJson(sur, output, BIG_BUF_SIZE);

            size_t written = flockfs.writeFile(surveyFileName, (uint8_t*)output, strlen(output));
            Serial.printf(CLI_CYA "Wrote " CLI_GRN "%d" CLI_CYA " bytes to file\r\n" CLI_RESET, written);
            flockLog.addLogLine("SCAN", "survey() wrote %d bytes to %s\r\n", written, surveyFileName);

            free (output);
        }
        else
        {
            flockLog.addLogLine("SCAN", "Unable to allocate %d bytes for file write buffer\r\n", BIG_BUF_SIZE + 1);
            Serial.printf(CLI_BOLD_RED "Unable to allocate for file write buffer!\r\n" CLI_RESET);
        }
    }
} // survey

/*************************************************
* What kind of RF signal?
*************************************************/
const char* discoveryToText(FLOCK_DISCOVERY_METHOD meth)
{
  switch (meth)
  {
    case NO_DISCOVERY:      return ("Unknown");
    case WIFI_DISCOVERY:    return ("WiFi");
    case BTLE_DISCOVERY:    return ("BTLE");
  }

  return ("");
}

/***************************************************
* WiFi Management packet subtype to human text
****************************************************/
const char* WiFiMgmtSubtypeToText(uint8_t stype)
{
  static char other[24];
  switch (stype)
  {
    case MGMT_FRAME_SUBTYPE_ASSOCIATION_REQUEST:      return ("associaton request");
    case MGMT_FRAME_SUBTYPE_ASSOCIATION_RESPONSE:     return ("association response");
    case MGMT_FRAME_SUBTYPE_REASSOCIATION_REQUEST:    return ("reassociation request");
    case MGMT_FRAME_SUBTYPE_REASSOCIATION_RESPONSE:   return ("reassociation response");
    case MGMT_FRAME_SUBTYPE_PROBE_REQUEST:            return ("probe request");
    case MGMT_FRAME_SUBTYPE_PROBE_RESPONSE:           return ("probe response");
    case MGMT_FRAME_SUBTYPE_TIMING_ADVERTISEMENT:     return ("timing advertisement");
    case MGMT_FRAME_SUBTYPE_BEACON:                   return ("beacon");
    case MGMT_FRAME_SUBTYPE_ATIM:                     return ("ATIM");
    case MGMT_FRAME_SUBTYPE_DISASSOCIATION:           return ("disassociation");
    case MGMT_FRAME_SUBTYPE_AUTHENTICATION:           return ("authentication");
    case MGMT_FRAME_SUBTYPE_DEAUTHENTICATION:         return ("deauthentication");
    case MGMT_FRAME_SUBTYPE_ACTION:                   return ("action");
    case MGMT_FRAME_SUBTYPE_ACTION_NO_ACK:            return ("action no-ack");
  }

  snprintf(other, 23, "Other: 0x%02x", stype);
  return (other);
}

/***************************************************
* WiFi Data packet subtype to human text
****************************************************/
const char* WiFiDataSubtypeToText(uint8_t stype)
{
  static char other[24];
  switch (stype)
  {
    case DATA_FRAME_SUBTYPE_DATA:                             return ("Data");
    case DATA_FRAME_SUBTYPE_DATA_CF_ACK:                      return ("Data CF-ACK");
    case DATA_FRAME_SUBTYPE_DATA_CF_POLL:                     return ("Data CF-Poll");
    case DATA_FRAME_SUBTYPE_DATA_CF_ACK_CF_POLL:              return ("Data CF-ACK + CF-Poll");
    case DATA_FRAME_SUBTYPE_DATA_NO_DATA_NULL:                return ("ND (null no data)");
    case DATA_FRAME_SUBTYPE_DATA_NO_DATA_CF_ACK:              return ("ND CF-ACK");
    case DATA_FRAME_SUBTYPE_DATA_NO_DATA_POLL:                return ("ND CF-Poll");
    case DATA_FRAME_SUBTYPE_DATA_NO_DATA_CF_ACK_CF_POLL:      return ("ND CF-ACK + CF-Poll");
    case DATA_FRAME_SUBTYPE_DATA_QOS_DATA:                    return ("QoS Data");
    case DATA_FRAME_SUBTYPE_DATA_QOS_CF_ACK:                  return ("QoS CF-ACK");
    case DATA_FRAME_SUBTYPE_DATA_QOS_CF_POLL:                 return ("QoS CF-Poll");
    case DATA_FRAME_SUBTYPE_DATA_QOS_CF_ACK_CF_POLL:          return ("QoS CF-ACK + CF-Poll");
    case DATA_FRAME_SUBTYPE_DATA_QOS_NO_DATA:                 return ("ND QoS");
    case DATA_FRAME_SUBTYPE_DATA_RESERVED_1:                  return ("Reserved");
    case DATA_FRAME_SUBTYPE_DATA_NO_DATA_QOS_POLL:            return ("ND QoS CF-Poll");
    case DATA_FRAME_SUBTYPE_DATA_NO_DATA_QOS_CF_ACK_CF_POLL:  return ("ND QoS CF-ACK + CF-Poll");
  }

  snprintf(other, 23, "Other: 0x%02x", stype);
  return (other);
}

/**************************************************
* WiFi type to text ("Management" or "Data")
**************************************************/
const char* wifiPktTypeToText(enum wifi_pkt_t t)
{
  switch (t)
  {
    case wifi_management: return ("Management");
    case wifi_data: return ("Data");
  }
  return ("unknown");
}

const char* btleAdvertisedTypeToText(uint8_t type)
{
  return ("test");
}

/************************************************
* Make a human-readable hex string from a MAC
* address (00:11:22:aa:bb:cc)
************************************************/
const char* macToText(const uint8_t* mac)
{
  static char ret[20];
  if (!mac)
  {
    return (NULL);
  }

  snprintf(ret, 19, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  return (ret);
}

/**************************************************
* Reverse byte order in a fixed size array of bytes
**************************************************/
void reverseBytes(uint8_t* data, size_t len)
{
  if (!data || !len)
  {
    return;
  }

  uint8_t* tmp = (uint8_t*)ps_malloc(len);
  if (!tmp)
  {
    return;
  }

  memcpy(tmp, data, len);
  size_t inx = 0;
  while(--len)
  {
    data[inx++] = tmp[len];
  }

  data[inx] = tmp[0];

  free(tmp);
}
