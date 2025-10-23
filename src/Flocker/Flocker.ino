#include "gps.h"

#define GPS_PORT_TX 6
#define GPS_PORT_RX 5
#define GPS_PORT_BAUD 9600

#define USER_LED 21

NMEAGPS gps;

void setup() {
  Serial.begin(112500); // init USB serial
  delay(1000);

  pinMode(USER_LED, OUTPUT);

  if (!gps.begin(GPS_PORT_BAUD, GPS_PORT_RX, GPS_PORT_TX)) {
    Serial.printf("gps not init'd!\r\n");
  } 

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
  static bool led = false;
  static bool first = false;
  
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

  float tmpFlt;

  if (fixQuality > 0) {
    //float tmpFlt = gps.getCourse();
    //if (tmpFlt != course) {
    //  Serial.printf("new course %f\r\n", tmpFlt);
    //  course = tmpFlt;
    //}

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

  if ((millis() - msnow) > 1000) {
    msnow = millis();

    if (led) {
      digitalWrite(USER_LED, HIGH);
      led = false;
    } else {
      digitalWrite(USER_LED, LOW);
      led = true;
    }
  }  
}
