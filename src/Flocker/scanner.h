#ifndef SCANNER_H_
#define SCANNER_H_

#include <string>

#include "Arduino.h"
#include "alloc.h"

#define SSID_LEN  32  // 802.11 max SSID length

enum wifi_pkt_t
{
  wifi_management,
  wifi_data
};

// structure to hold details about every device found; not just
// Flock type things at this point (found by WiFi)
struct found_wifi_t
{
  wifi_pkt_t type;
  uint8_t subtype;
  uint8_t channel;
  char ssid[SSID_LEN + 1];
  uint8_t sourceAddr[6];
  uint8_t destAddr[6];
  uint32_t timestamp;
  int8_t rssi;
};

// structure to hold details about every device found; not just
// Flock type things at this point (found by Bluetooth LE)
struct found_ble_t
{
  char name[SSID_LEN + 1];
  uint8_t mac[6];
  int8_t rssi;
  uint32_t timestamp;
  uint16_t services16;
  uint16_t serviceData16;
  uint32_t services32;
  uint32_t serviceData32;
  flk::string services128;
  flk::string serviceData128;
};

class SCANNER
{
public:
  SCANNER() {}
  ~SCANNER() {}

  bool begin();
  void survey(uint32_t interval, const char* fname, const char* notes);
  void scan(const char* logname);
  void update();

private:
  void startWiFi();
  void startBLE();
  void stopBLE();
  void stopScanning();
  void postSurveyActivities();

  bool scannerRunning;
  bool surveyStopTrigger;
  char surveyFileName[120];
  char surveyNotes[120];
  uint32_t channelTime;
  uint32_t channelTimeOffset;
  uint32_t btTimeOffset;
};

#endif //SCANNER_H_
