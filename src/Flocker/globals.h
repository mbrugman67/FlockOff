#ifndef GLOBALS_H_
#define GLOBALS_H_

#include "gps.h"
#include "led.h"
#include "cli.h"
#include "flockFs.h"
#include "flockCfg.h"
#include "scanner.h"
#include "led.h"
#include "flockLog.h"


/**********************************
* Versions
**********************************/
#define FLOCKER_VERSION  "0.9a"
#define SURVEY_JSON_VERSION  2


/**********************************
* gobal classes/constants/variables
**********************************/
extern NMEAGPS gps;
extern MBFS flockfs;
extern CONFIG flockCfg;
extern SCANNER flockScan;
extern LEDS flockLED;
extern FLOGGER flockLog;
extern bool psRamInitOk;
extern bool initOk;


/**********************************
* global defines
**********************************/
#define SSID_LEN  32  // 802.11 max SSID length


/**********************************
# enum discovery method
**********************************/
enum FLOCK_DISCOVERY_METHOD
{
  NO_DISCOVERY = 0,
  WIFI_DISCOVERY,
  BTLE_DISCOVERY
};

const char* discoveryToText(FLOCK_DISCOVERY_METHOD meth);
const char* mgmtSubtypeToText(uint8_t stype);
const char* macToText(const uint8_t* mac);

/**********************************
* #defines for CLI/serial output
* colors (macros for ANSII codes)
**********************************/
#define CLI_RESET "\x1b[0m"
#define CLI_CLEAR "\x1b[2J"
#define CLI_BACKSPACE "\x1b[D"
#define CLI_DELETE "\x1B[P"

#define CLI_BLK "\x1b[30m"
#define CLI_RED "\x1b[31m"
#define CLI_GRN "\x1b[32m"
#define CLI_YEL "\x1b[33m"
#define CLI_BLU "\x1b[34m"
#define CLI_PUR "\x1b[35m"
#define CLI_CYA "\x1b[36m"
#define CLI_WHT "\x1b[37m"

#define CLI_BOLD "\x1b[1m"
#define CLI_BOLD_BLK "\x1b[1;30m"
#define CLI_BOLD_RED "\x1b[1;31m"
#define CLI_BOLD_GRN "\x1b[1;32m"
#define CLI_BOLD_YEL "\x1b[1;33m"
#define CLI_BOLD_BLU "\x1b[1;34m"
#define CLI_BOLD_PUR "\x1b[1;35m"
#define CLI_BOLD_CYA "\x1b[1;36m"
#define CLI_BOLD_WHT "\x1b[1;37m"


#define FORMAT_LITTLEFS_IF_FAILED true

#endif // GLOBALS_H_
