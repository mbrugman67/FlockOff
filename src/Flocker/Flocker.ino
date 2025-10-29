#include "gps.h"
#include "led.h"

#define GPS_PORT_TX 6
#define GPS_PORT_RX 5
#define GPS_PORT_BAUD 9600

#define USER_LED 21

#define CLED_R 7
#define CLED_G 9
#define CLED_B 8

NMEAGPS gps;
LEDS commLeds;

void setup() {
  pinMode(USER_LED, OUTPUT);
  Serial.begin(112500); // init USB serial

  if (!gps.begin(GPS_PORT_BAUD, GPS_PORT_RX, GPS_PORT_TX)) {
    Serial.printf("gps not init'd!\r\n");
  } 

  commLeds.begin(CLED_R, CLED_G, CLED_B);

  delay(1000);

  //gps.setDebug(true);
  Serial.printf("\r\n <<<<<<< STARTING >>>>>>>\r\n");
}


void printDate() {
  struct tm tm_;
  gps.getTime(&tm_);

  Serial.printf("%02d:%02d:%02d  %d/%d/%d\r\n", 
      tm_.tm_hour, tm_.tm_min, tm_.tm_sec,
      tm_.tm_mon, tm_.tm_mday, tm_.tm_year + 1900);
}


void loop() {
  static uint32_t msnow = millis();
  static uint32_t ledOffset = millis();
  static uint8_t ledState = 0;
  
  static float lat = -999.9;
  static float lng = -999.9;
  static float speed = -999.9;
  static float course = -999.9;
  static int fixQuality = -1;

  int tmpInt = gps.getFixQuality();
  if (tmpInt != fixQuality) {
    Serial.printf("new fixqual %d\r\n", tmpInt);
    fixQuality = tmpInt;
  }

  if (fixQuality > 0) {
    float tmpFlt;
    bool changed = false;

    tmpFlt = gps.getLatitude();
    if (tmpFlt != lat) {
      Serial.printf("new latitude %f\r\n", tmpFlt);
      lat = tmpFlt;
      changed = true;
    }

    tmpFlt = gps.getLongitude();
    if (tmpFlt != lng) {
      Serial.printf("new longitude %f\r\n", tmpFlt);
      lng = tmpFlt;
      changed = true;
    }

    tmpFlt = gps.getSpeed();
    if (fabs(tmpFlt - speed) > 0.5) {
      Serial.printf("new speed %f\r\n", tmpFlt);
      speed = tmpFlt;
      changed = true;
    }

    if (changed) {
      printDate();
      Serial.printf("\r\n");
    }
  }

  gps.update();

  if ((millis() - msnow) > 500) {
    msnow = millis();
    digitalWrite(USER_LED, !digitalRead(USER_LED));
  }

  switch (ledState)
  {
    case 0:
    {
      commLeds.cycleRed(2000, 1500);
      ledOffset = millis();
      ledState = 1;
    }  break;

    case 1:
    {
      if (millis() - ledOffset > 1000)
      {
        commLeds.cycleGrn(2000, 1500);
        ledOffset = millis();
        ledState = 2;
      }
    }  break;

    case 2:
    {
      if (millis() - ledOffset > 1000)
      {
        commLeds.cycleBlu(2000, 1500);
        ledOffset = millis();
        ledState = 3;
      }  
    }  break;

    case3:
    {

    }  break;
  }

  commLeds.update();
}
