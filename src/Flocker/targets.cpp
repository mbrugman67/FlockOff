#include "targets.h"

#include <cctype>

#include "globals.h"
#include "flockFs.h"
#include "defaultTargets.h"

#define FILE_WIFI_MAC "wifiMacs.cfg"
#define FILE_WIFI_NAME "wifiNames.cfg"
#define FILE_BT_MAC "btleMacs.cfg"
#define FILE_BT_NAME "btleNames.cfg"
#define FILE_BT_UUID16 "btleUUID16.cfg"

#define TARGET_INPUT_STR_LEN 80

bool TARGETS::begin()
{
  inputstr = (char*)ps_malloc(TARGET_INPUT_STR_LEN + 1);
  this->loadDefaultWiFiMacs();
  return (inputstr);
}

wifi_match_t TARGETS::isWiFiMatch(const found_wifi_t& w, flk::string& info)
{
  char prefix[9] = {'\0'};
  char PREFIX[9] = {'\0'};
  strncpy(prefix, macToText(w.sourceAddr), 8);

  for (uint8_t ii = 0; ii < 9; ++ii)
  {
    PREFIX[ii] = std::toupper(prefix[ii]);
  }

  citWiFiMacs = wiFiMacs.find(PREFIX);

  if (citWiFiMacs != wiFiMacs.end())
  {
    info = flk::string(citWiFiMacs->second);
    return (WIFI_MATCH_MAC);
  }

  if (strlen(w.ssid))
  {
    size_t len = sizeof(wiFiDefNames) / sizeof(wiFiDefNames[0]);
    for (size_t ii = 0; ii < len; ++ii)
    {
      if (strcasestr(wiFiDefNames[ii], w.ssid))
      {
        info = flk::string(wiFiDefNames[ii]);
        return (WIFI_MATCH_SSID);
      }
    }
  }

  return (WIFI_MATCH_NONE);
}

bt_match_t TARGETS::isBTMatch(const found_ble_t& b, flk::string& info)
{
    char tmp[128] = {'\0'};

    if (b.services32)
    {
        snprintf(tmp, 127, "ServiceUUID 0x%04x", b.services32);
        info = flk::string(tmp);
        return (BT_MATCH_UUID32);
    }

    if (b.serviceData32)
    {
        snprintf(tmp, 127, "ServiceDataUUID 0x%04x", b.serviceData32);
        info = flk::string(tmp);
        return (BT_MATCH_UUID32);
    }

    if (!strncasecmp(macToText(b.mac), "8c:ea:48", 8))
    {
      info = flk::string("8c:ea:48");
      return (BT_MATCH_MAC);
    }

    return (BT_MATCH_NONE);
}

int TARGETS::loadDefaultWiFiMacs()
{
  wiFiMacs.clear();

  size_t defaultCount = (sizeof(wifiDefMacs) / sizeof(match_mac_t));

  flockLog.addLogLine("TARGETS", "Adding %d default WiFi MAC match prefixes:\r\n", defaultCount);

  for (size_t ii = 0; ii < defaultCount; ++ii)
  {
    wiFiMacs.insert(std::make_pair(wifiDefMacs[ii].prefix, wifiDefMacs[ii].name));
    flockLog.addLogLine("TARGETS", "  -->%s - %s\r\n", wifiDefMacs[ii].prefix, wifiDefMacs[ii].name);
    if (!(ii % 20))
    {
      flockLog.flushNow();
    }
  }

  flockLog.addLogLine("TARGETS", "Finished adding %d default MAC match prefixes.\r\n", wiFiMacs.size());

  return (defaultCount);
}


/*****************************************************
 * Read a string from serial and convert to an integer
******************************************************/
int TARGETS::readInt(const char* prompt)
{
  this->readString(prompt);
  return ((int)strtol(inputstr, NULL, 10));
}

/*****************************************************
* Read a single char from serial
******************************************************/
char TARGETS::readChar(const char* prompt)
{
  static char c;

  Serial.printf(prompt);
  holdCLI(true);

  while (!Serial.available())
  {
    flockLED.update();
  }

  c = Serial.read();
  holdCLI(false);

  return (c);
}

/*****************************************************
* Read a string
******************************************************/
const char* TARGETS::readString(const char* prompt)
{
  int posn = 0;

  Serial.printf(prompt);
  holdCLI(true);

  while (posn < TARGET_INPUT_STR_LEN)
  {
    while (!Serial.available())
    {
      flockLED.update();
    }

    char c = Serial.read();
    if (c == '\r')
    {
      break;
    }
    else if (c == '\b' || c == 0x7f)
    {
      if (posn)
      {
        Serial.printf("\b \b");
        --posn;
      }
    }
    else
    {
      if (isprint(c))
      {
        inputstr[posn] = c;
        Serial.print(c);
        ++posn;
      }
      else
      {
        Serial.printf(" >>0x%2x<< ", c);
      }
    }
  }

  inputstr[posn] = '\0';

  Serial.printf("\r\n");
  holdCLI(false);

  return (inputstr);
}
