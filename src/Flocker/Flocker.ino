/***********************************************************************
* Main entry point for the flocker.
***********************************************************************/
#include "gps.h"
#include "led.h"
#include "cli.h"
#include "flockFs.h"
#include "flockCfg.h"
#include "scanner.h"
#include "flockLog.h"

// constants for config'ing objects
#define GPS_PORT_TX 5        // defined by circuit board - don't change unless you changed hardware
#define GPS_PORT_RX 6        // defined by circuit board - don't change unless you changed hardware
#define GPS_PORT_BAUD 9600   // baud rate for GPS module - don't change unless you're using a different module
#define ADDR_LED_PIN 9       // defined by circuit board - don't change unless you changed hardware
#define USER_LED 21          // built-in user LED on XIAO ESP32S3 module

// global class instances
FLOGGER flockLog;
NMEAGPS gps;
MBFS flockfs;
CONFIG flockCfg;
SCANNER flockScan;
LEDS flockLED;
bool psRamInitOk;
bool initOk;

static uint8_t cfgListenerID = BAD_LISTENER_ID;
static uint32_t bootTick;
static bool allowAutoScan = true;

// if you see this message on the CLI, you messed up an allocation!
void heapCheck() {
  if (!heap_caps_check_integrity_all(true)) {
    Serial.printf("HEAPCHK->HEAPS FUCKED!\r\n");
  }
}

// Arduino setup method - called once by embedded OS on startup
void setup() {
  initOk = true;
  psRamInitOk = psramInit();
  pinMode(USER_LED, OUTPUT);
  Serial.begin(112500);  // init USB serial
  delay(2000);

  // order is important here - don't change this order!
  initOk &= gps.begin(GPS_PORT_BAUD, GPS_PORT_RX, GPS_PORT_TX);
  initOk &= flockfs.begin();
  initOk &= flockCfg.begin();
  initOk &= flockLog.begin(200, "system", flockCfg.getDebugFileCount());
  initOk &= flockScan.begin();
  initOk &= flockLED.begin(ADDR_LED_PIN, 140);
  initOk &= setupCLI();

  if (!initOk)
  {
    delay(2000);
    Serial.printf("SOMETHING DIDN'T INIT.  Try turning debug logging on :/\r\n");
    delay(2000);
  }
  else
  {
    flockCfg.registerListener(cfgListenerID);
  }

  bootTick = millis();

  flockLog.addLogLine("main", "Leaving setup()\r\n");
}

// Arduino-defined main loop.  Called continuously by embedded OS
void loop() {
  static uint32_t msFlash = millis();
  static uint32_t msHeap = millis() - 1000;

  // update classes
  if (initOk)
  {
    gps.update();
    flockLED.update();
    updateCLI();
    flockLog.update();
    flockScan.update();

    // if no CLI action seen in the first 5 seconds,
    // start a continuous scan process
    if (allowAutoScan)
    {
      if (cliActive())
      {
        allowAutoScan = false;
      }

      if ((millis() - bootTick) > 5000)
      {
        flockScan.scan("");
        allowAutoScan = false;
      }
    }
  }

  // user LED continually flashes - visual heartbeat
  if ((millis() - msFlash) > 500) {
    msFlash = millis();
    digitalWrite(USER_LED, !digitalRead(USER_LED));
  }

  // check the stack every 2 seconds
  if ((millis() - msHeap) > 2000) {
    msHeap = millis();
    heapCheck();
  }
}
