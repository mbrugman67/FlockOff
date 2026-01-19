/********************************************************************
 * flockCfg.h
 ********************************************************************
 * Declaration of the config class.  Configuration is stored
 * in a JSON file.  This will handle reading/writing/backing up
 * the config file.
 *
 * Also, there's a handler for setting config values over the CLI
 *******************************************************************/
#ifndef CFG_H_
#define CFG_H_

#include <ArduinoJson.h>

#define MAX_LISTENERS 10
#define BAD_LISTENER_ID 255
#define INPUT_STR_LEN 80

class CONFIG
{
public:
  CONFIG() {}
  ~CONFIG() {}

  bool begin();
  bool buildDefualtConfig();
  void outputJson();
  void setConfigValues();
  void setTimeZone();
  const char* getTimeZone();
  const char* getDeviceName();
  uint8_t getLEDBrightness();
  bool getDebugEnabledState();
  bool getScanLogEnabledState();
  uint8_t getDebugFileCount();
  uint8_t getScanLogFileCount();
  uint16_t getScanHoldTime();
  int8_t getMinRSSI();

  bool newCfgAvailable(uint8_t listenerID);
  bool registerListener(uint8_t& id);

private:
  void setLEDBrightness();
  void selectTimeZone();
  void setDeviceName();
  void setDebugEnabledState();
  void setScanLogEnabledState();
  void setDebugFileRollCount();
  void setScanLogRollCount();
  void setScanHoldTime();
  bool writeConfig();
  void setMinRSSI();
  void setNewConfigFlags(bool val);

  int readInt(const char* prompt);
  char readChar(const char* prompt);
  const char* readString(const char* prompt);

  bool hasNewConfig[MAX_LISTENERS];
  uint8_t nextListenerID;
  JsonDocument cfg;

  char inputstr[INPUT_STR_LEN + 1];

};

#endif //CFG_H_
