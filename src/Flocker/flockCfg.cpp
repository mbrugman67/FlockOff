/********************************************************************
 * flockCfg.cpp
 ********************************************************************
 * Implementation of the config class.  Configuration is stored
 * in a JSON file.  This will handle reading/writing/backing up
 * the config file.
 *
 * Also, there's a handler for setting config values over the CLI
 *******************************************************************/
#include <Arduino.h>

#include "flockCfg.h"
#include "led.h"
#include "flockLog.h"

#include "globals.h"

// JSON keys for config set
#define DEVICENAME "deviceName"
#define LEDBRIGHTNESS "LEDBrightness"
#define TIMEZONE "timeZone"
#define WIFIAPS "WifiAPs"
#define DEBUGENABLED "debugEnabled"
#define DEBUGROLLCOUNT "debugLogRollCount"
#define MINRSSI "minRSSI"

// name of configuration file
#define CONFIG_FILENAME "config.json"
#define CONFIG_BACKUP_FILENAME "config.bak"

// default WiFi names corresponding to know surveillance devices.  This
// list is the initial - more can be added later by the user
const char* defaultWiFiAPs[] = {"flock", "fs ext Battery", "penguin", "pigvision"};

// Struct for following list of timezones
struct cfg_tz_t
{
    uint8_t inx;
    const char* desc;
    const char* tz;
};

// List of timezones to select from.  This is a very US-centric
// list, with default "offset from GMT" zones included
const cfg_tz_t zones[] = {{0, "default CSD/CDT", "CST6CDT,M3.2.0,M11.1.0"},
                          {1, "America/Anchorage", "AKST9AKDT,M3.2.0,M11.1.0"},
                          {2, "America/Chicago", "CST6CDT,M3.2.0,M11.1.0"},
                          {3, "America/Denver", "MST7MDT,M3.2.0,M11.1.0"},
                          {4, "America/Detroit", "EST5EDT,M3.2.0,M11.1.0"},
                          {5, "America/Indiana/Indianapolis", "EST5EDT,M3.2.0,M11.1.0"},
                          {6, "America/Indiana/Knox", "CST6CDT,M3.2.0,M11.1.0"},
                          {7, "America/Juneau", "AKST9AKDT,M3.2.0,M11.1.0"},
                          {8, "America/Los_Angeles", "PST8PDT,M3.2.0,M11.1.0"},
                          {9, "America/New_York", "EST5EDT,M3.2.0,M11.1.0"},
                          {10, "America/Phoenix", "MST7"},
                          {11, "Pacific/Honolulu", "HST10"},
                          {12, "Etc/Greenwich", "GMT0"},
                          {13, "Etc/GMT", "GMT0"},
                          {14, "Etc/GMT-0", "GMT0"},
                          {15, "Etc/GMT-1", "<+01>-1"},
                          {16, "Etc/GMT-2", "<+02>-2"},
                          {17, "Etc/GMT-3", "<+03>-3"},
                          {18, "Etc/GMT-4", "<+04>-4"},
                          {19, "Etc/GMT-5", "<+05>-5"},
                          {20, "Etc/GMT-6", "<+06>-6"},
                          {21, "Etc/GMT-7", "<+07>-7"},
                          {22, "Etc/GMT-8", "<+08>-8"},
                          {23, "Etc/GMT-9", "<+09>-9"},
                          {24, "Etc/GMT-10" ,"<+10>-10"},
                          {25, "Etc/GMT-11" ,"<+11>-11"},
                          {26, "Etc/GMT-12" ,"<+12>-12"},
                          {27, "Etc/GMT-13" ,"<+13>-13"},
                          {28, "Etc/GMT-14" ,"<+14>-14"},
                          {29, "Etc/GMT0"," GMT0"},
                          {30, "Etc/GMT+0", "GMT0"},
                          {31, "Etc/GMT+1", "<-01>1"},
                          {32, "Etc/GMT+2", "<-02>2"},
                          {33, "Etc/GMT+3", "<-03>3"},
                          {34, "Etc/GMT+4", "<-04>4"},
                          {35, "Etc/GMT+5", "<-05>5"},
                          {36, "Etc/GMT+6", "<-06>6"},
                          {37, "Etc/GMT+7", "<-07>7"},
                          {38, "Etc/GMT+8", "<-08>8"},
                          {39, "Etc/GMT+9", "<-09>9"},
                          {40, "Etc/GMT+10", "<-10>10"},
                          {41, "Etc/GMT+11", "<-11>11"},
                          {42, "Etc/GMT+12", "<-12>12"}};

const size_t tzCount = sizeof(zones) / sizeof(cfg_tz_t);

/******************************************************
* Constructor
******************************************************/
bool CONFIG::begin()
{
  this->setNewConfigFlags(false);
  nextListenerID = 0;

  // check for a config file.  If it doesn't exist, create defaults
  if (!flockfs.fileExists(CONFIG_FILENAME))
  {
    Serial.print(CLI_BOLD_RED "CONFIG::begin()-> Built default config data\r\n" CLI_RESET);
    return (this->buildDefualtConfig());
  }
  else
  {
    // file exists.  Load from filesystem and create the JSON structure from it
    size_t cfgSize = flockfs.getFileSize(CONFIG_FILENAME);

    // allocate spacee
    char* json = (char*)ps_malloc(cfgSize);
    if (!json)
    {
      Serial.print(CLI_BOLD_RED "CONFIG::begin()-> Did not allocate to build default config data\r\n" CLI_RESET);
      return (false);
    }

    // read the file
    if (flockfs.readFile(CONFIG_FILENAME, (uint8_t*)json, cfgSize) == -1)
    {
      Serial.printf(CLI_BOLD_RED "CONFIG::begin()-> Error reading config file\r\n" CLI_RESET);
      return (false);
    }

    // conver to JSON structure
    deserializeJson(cfg, json);

    // free memory
    delete (json);

    // tell the world there is new config data available
    this->setNewConfigFlags(true);
  }

  return(true);
}

/******************************************************
* Create a new JSON config structure from defaults
* and save it to file
******************************************************/
bool CONFIG::buildDefualtConfig()
{
  flockfs.deleteFile(CONFIG_FILENAME);

  cfg[DEVICENAME] = "Generic ESP32S3";         // tag that can be added to data
  cfg[TIMEZONE] = "CST6CDT,M3.2.0,M11.1.0";    // my timezone :)
  cfg[LEDBRIGHTNESS] = 128;                    // max LED brightness when flashing (0-255)
  cfg[DEBUGENABLED] = false;                   // debug logging enabled?
  cfg[DEBUGROLLCOUNT] = 3;                     // how many previous debug files to save
  cfg[MINRSSI] = -85;                          // minimum signal strength for alert

  JsonArray wifiAPs = cfg[WIFIAPS].to<JsonArray>();

  size_t dfltAPCount = sizeof(defaultWiFiAPs) / sizeof(*defaultWiFiAPs);

  for(size_t ii = 0; ii < dfltAPCount; ++ii)
  {
    wifiAPs.add(defaultWiFiAPs[ii]);
  }

  this->setNewConfigFlags(true);
  return (this->writeConfig());
}

/******************************************************
* Pretty-print JSON config structure
******************************************************/
void CONFIG::outputJson()
{
  Serial.printf(CLI_CYA);
  serializeJsonPretty(cfg, Serial);
  Serial.printf("\r\n" CLI_CYA);
}

/*****************************************************
 * Read a string from serial and convert to an integer
******************************************************/
int CONFIG::readInt(const char* prompt)
{
  this->readString(prompt);
  return ((int)strtol(inputstr, NULL, 10));
}

/*****************************************************
* Read a single char from serial
******************************************************/
char CONFIG::readChar(const char* prompt)
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
const char* CONFIG::readString(const char* prompt)
{
  int posn = 0;

  Serial.printf(prompt);
  holdCLI(true);

  while (posn < INPUT_STR_LEN)
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

void CONFIG::setConfigValues()
{
  bool configDone = false;

  while (!configDone)
  {
    Serial.printf(CLI_YEL "1) Set device name (" CLI_BOLD_GRN "%s" CLI_RESET CLI_YEL ")\r\n" CLI_RESET, this->getDeviceName());
    Serial.printf(CLI_YEL "2) Set timezone (" CLI_BOLD_GRN "%s" CLI_RESET CLI_YEL ")\r\n" CLI_RESET, this->getTimeZone());
    Serial.printf(CLI_YEL "3) Set max LED brightness (" CLI_BOLD_GRN "%d" CLI_RESET CLI_YEL ")\r\n" CLI_RESET, (int)this->getLEDBrightness());
    Serial.printf(CLI_YEL "4) Set debug logging (" CLI_BOLD_GRN "%s" CLI_RESET CLI_YEL ")\r\n" CLI_RESET, this->getDebugEnabledState() ? "enabled" : "disabled");
    Serial.printf(CLI_YEL "5) Set debug file rolling count (" CLI_BOLD_GRN "%d" CLI_RESET CLI_YEL ")\r\n" CLI_RESET, (int)this->getDebugFileCount());
    Serial.printf(CLI_YEL "6) Set minimum RSSI (" CLI_BOLD_GRN "%d" CLI_RESET CLI_YEL ")\r\n" CLI_RESET, (int)this->getMinRSSI());

    char choice = this->readChar((CLI_CYA "Select number of item to change or 'x' to exit with no changes: " CLI_RESET));
    Serial.printf("\r\n\r\n");

    switch (choice)
    {
      case '1':  this->setDeviceName(); Serial.printf("\r\n"); break;
      case '2':  this->selectTimeZone(); Serial.printf("\r\n"); break;
      case '3':  this->setLEDBrightness(); Serial.printf("\r\n"); break;
      case '4':  this->setDebugEnabledState(); Serial.printf("\r\n"); break;
      case '5':  this->setDebugFileRollCount(); Serial.printf("\r\n"); break;
      case '6':  this->setMinRSSI(); Serial.printf("\r\n"); break;
      case 'x':  configDone = true; break;
      default: Serial.printf(CLI_BOLD_RED "Unknown entry '%c'\r\n" CLI_RESET, choice);
    }
  }
}

/******************************************************
* Set the timezone.  This is a menu-driven thing, it
* takes the serial input away from the default CLI
* handler and returns it later.  Will save updated
* timezone to file
******************************************************/
void CONFIG::selectTimeZone()
{
  // print all of the available timezones in two columns
  for (size_t ii = 1; ii < tzCount / 2; ++ii)
  {
    Serial.printf(CLI_YEL " %2d)" CLI_BOLD_GRN " %-30s " CLI_RESET " " CLI_YEL " %2d) " CLI_BOLD_GRN "%s\r\n"
        CLI_RESET, ii, zones[ii].desc, ii + 21, zones[ii + 21].desc);
  }

  int selected = this->readInt((CLI_CYA "Select timezone by number: " CLI_RESET));
  if (selected > tzCount) selected = 0;   // sanity check

  cfg[TIMEZONE] = zones[selected].tz;
  flockLog.addLogLine("CFG", "selectTimeZone() set to %s\r\n", zones[selected].tz);
  this->setNewConfigFlags(true);
  this->setTimeZone();
  this->writeConfig();
}

/******************************************************
* Set the actual timezone to the internal OS/runtime
******************************************************/
void CONFIG::setTimeZone()
{
  const char* tz = cfg[TIMEZONE];
  setenv("TZ", tz, 1);
  tzset();
}

/******************************************************
* Set the minimum RSSI
******************************************************/
void CONFIG::setMinRSSI()
{
  bool goodRSSIEntered = false;

  while (!goodRSSIEntered)
  {
    int  RSSI = this->readInt((CLI_CYA "Enter new minimum RSSI (-90 to -10): " CLI_RESET));

    if (RSSI >= -90 && RSSI <= -10)
    {
      goodRSSIEntered = true;
      cfg[MINRSSI] = (int8_t)RSSI;
      flockLog.addLogLine("CFG", "setMinRSSI() set to %d\r\n", RSSI);
    }
  }

  this->setNewConfigFlags(true);
  this->writeConfig();
}

/******************************************************
* Get the minimium RSSI
******************************************************/
int8_t CONFIG::getMinRSSI()
{
  return ((int8_t)cfg[MINRSSI]);
}

/******************************************************
* Set the device name
******************************************************/
void CONFIG::setDeviceName()
{
  this->readString((CLI_CYA "Enter new device name: " CLI_RESET));
  cfg[DEVICENAME] = inputstr;
  flockLog.addLogLine("CFG", "setDeviceName() set to %s\r\n", inputstr);
  this->setNewConfigFlags(true);
  this->writeConfig();
}

/******************************************************
* Get the device name.
******************************************************/
const char* CONFIG::getDeviceName()
{
  return (cfg[DEVICENAME]);
}

/******************************************************
* Set the LED brightness.  This is a menu-driven thing, it
* takes the serial input away from the default CLI
* handler and returns it later.  Will save updated
* LED brightness to file
******************************************************/
void CONFIG::setLEDBrightness()
{
  bool goodBrightEntered = false;

  while (!goodBrightEntered)
  {
    int  bright = this->readInt((CLI_CYA "Enter new LED max brightness (0 - 255): " CLI_RESET));

    if (bright >= 0 && bright <= 255)
    {
      goodBrightEntered = true;
      cfg[LEDBRIGHTNESS] = (uint8_t)bright;
      flockLog.addLogLine("CFG", "setLEDBrightness() set to %d\r\n", bright);

      if (bright == 0)
      {
        Serial.printf(CLI_YEL "Brightness is set to zero, which results in LEDs never lighting.\r\n" CLI_RESET);
      }
    }
  }

  this->setNewConfigFlags(true);
  this->writeConfig();
}

/******************************************************
* Get the LED brightness
******************************************************/
uint8_t CONFIG::getLEDBrightness()
{
  return (cfg[LEDBRIGHTNESS]);
}

/******************************************************
* Get the timezone.
******************************************************/
const char* CONFIG::getTimeZone()
{
  return (cfg[TIMEZONE]);
}

/******************************************************
* Get the debug enabled state.  If true, the system
* will be saving debug information to a log file in
* internal filesystem
******************************************************/
bool CONFIG::getDebugEnabledState()
{
  return (cfg[DEBUGENABLED]);
}

/******************************************************
* Set the debug enabled state.  If true, the system
* will be saving debug information to a log file in
* internal filesystem
******************************************************/
void CONFIG::setDebugEnabledState()
{
  bool done = false;

  while (!done)
  {
    char enb = this->readChar((CLI_CYA "'Y' to enable debug, 'N' to disable debug logging: " CLI_RESET));
    if (enb == 'Y' || enb == 'y')
    {
      cfg[DEBUGENABLED] = true;
      done = true;
    }
    else if (enb == 'N' || enb == 'n')
    {
      cfg[DEBUGENABLED] = false;
      done = true;
    }
  }

  this->setNewConfigFlags(true);
  this->writeConfig();
}

/******************************************************
* Get the debug log file count.  When enabled, the
* system will open a new debug log file and "roll"
* any existing files.  This config value controls how
* many previous files to keep
******************************************************/
uint8_t CONFIG::getDebugFileCount()
{
  return ((uint8_t)cfg[DEBUGROLLCOUNT]);
}

/******************************************************
* Set the debug log file count.  When enabled, the
* system will open a new debug log file and "roll"
* any existing files.  This config value controls how
* many previous files to keep
******************************************************/
void CONFIG::setDebugFileRollCount()
{

  bool done = false;

  while (!done)
  {
    int count = this->readInt((CLI_CYA "Enter number of debug log files to keep (1 - 10): " CLI_RESET));
    if (count > 0 && count < 11)
    {
      cfg[DEBUGROLLCOUNT] = count;
      flockLog.addLogLine("CFG", "setDebugFileRollCount() set to %d\r\n", count);
      done = true;
    }
  }

  this->setNewConfigFlags(true);
  this->writeConfig();
}


/******************************************************
* Write the JSON config structure to file (minimized).
* If there is an existing JSON file, it wall be renamed
* as a backup
******************************************************/
bool CONFIG::writeConfig()
{
  bool retVal = false;
  char* cfgjson = (char*)ps_malloc(2048);

  if (!cfgjson)
  {
    Serial.printf(CLI_BOLD_RED "CONFIG::writeConfig()->DID NOT ALLOCATE WRITE BUFFER" CLI_RESET);
    return(retVal);
  }

  flockfs.renameFile(CONFIG_FILENAME, CONFIG_BACKUP_FILENAME);

  memset(cfgjson, 0, 2048);
  serializeJson(cfg, cfgjson, 2047);

  if (flockfs.writeFile(CONFIG_FILENAME, (uint8_t*)cfgjson, strlen(cfgjson)) == -1)
  {
    Serial.printf(CLI_BOLD_RED "CONFIG::writeConfig()-> Error writing config file\r\n" CLI_RESET);
  }
  else
  {
    retVal = true;
  }

  free(cfgjson);

  if (retVal)
  {
    this->setNewConfigFlags(true);
  }

  return (retVal);
}

/******************************************************
*******************************************************
*******************************************************
* The next few methods handle letting clients know
* that there has been a change to configuration data.
*
* Each client registers its desire to be alerted of
* config changes and receives a "new config ID".  That
* client can then periodically poll using that ID to see
* if there have been changes
*******************************************************
*******************************************************
******************************************************/

/******************************************************
* Set all listener's flags to desired value.  Typically
* set all to true on a change of config
******************************************************/
void CONFIG::setNewConfigFlags(bool val)
{
  for (uint8_t ii = 0; ii < MAX_LISTENERS; ++ii)
  {
    hasNewConfig[ii] = val;
  }
}

/******************************************************
* Poll for change using client's listener ID. Will
* return true if there has been a change.  This is a
* destructive read - once read, that client's flag
* will be set to false
******************************************************/
bool CONFIG::newCfgAvailable(uint8_t listenerID)
{
  bool retval;
  if (listenerID >= MAX_LISTENERS || listenerID == BAD_LISTENER_ID)
  {
    return (false);
  }

  retval = hasNewConfig[listenerID];
  hasNewConfig[listenerID] = false;

  return (retval);
}

/******************************************************
* Method for a client to register as a config change
* listener.  Their ID will be returned, as well as
* a result bool
*
* If all client slots have been filled, the method
* will return false and set the ID to the known "BAD"
* value.
*
* If a slot is available, the client will get a unique
* ID and the method will return true
******************************************************/
bool CONFIG::registerListener(uint8_t& id)
{
  if (nextListenerID == MAX_LISTENERS)
  {
    id = BAD_LISTENER_ID;
    return (false);
  }

  id = nextListenerID++;
  return (true);
}
