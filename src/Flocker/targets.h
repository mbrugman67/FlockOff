#ifndef TARGETS_H_
#define TARGETS_H_

#include <map>
#include "alloc.h"
#include "scanner.h"

enum wifi_match_t
{
    WIFI_MATCH_NONE = 0,
    WIFI_MATCH_MAC,
    WIFI_MATCH_SSID
};

enum bt_match_t
{
  BT_MATCH_NONE = 0,
  BT_MATCH_MAC,
  BT_MATCH_NAME,
  BT_MATCH_UUID16
};

class TARGETS
{
public:
  TARGETS() {}
  ~TARGETS() {}

  bool begin();

  int loadDefaultWiFiMacs();
  int getWiFiMacCount()  { return (wiFiMacs.size()); }

  wifi_match_t isWiFiMatch(const found_wifi_t& w, utils::string& info);
  bt_match_t isBTMatch(const found_ble_t& b);

private:
  char* inputstr;
  int readInt(const char* prompt);
  char readChar(const char* prompt);
  const char* readString(const char* prompt);
  bool saveAllFiles();

  std::map<const char*, const char*, cmp_const_char, psramAlloc<std::map<const char*, const char*>::value_type>> wiFiMacs;
  std::map<const char*, const char*>::const_iterator citWiFiMacs;
};

#endif // TARGETS_H_