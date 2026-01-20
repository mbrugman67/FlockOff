#include "targets.h"

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
  return (true);
}

wifi_match_t TARGETS::isWiFiMatch(const found_wifi_t& w)
{
  return (WIFI_MATCH_NONE);
}

bt_match_t TARGETS::isBTMatch(const found_ble_t& b)
{
  return (BT_MATCH_NONE);
}

int TARGETS::loadDefaultWiFiMacs()
{
  wiFiMacs.clear();

  size_t defaultCount = (sizeof(wifi_macs) / sizeof(wifi_macs[0]));

  flockLog.addLogLine("TARGETS", "Adding %d default WiFi MAC match prefixes:\r\n", defaultCount);
  for (size_t ii = 0; ii < defaultCount; ++ii)
  {
    wiFiMacs.push_back(std::string(wifi_macs[ii]));
    flockLog.addLogLine("TARGETS", "  -->%s\r\n", wifi_macs[ii]);
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





