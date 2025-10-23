#ifndef GPS_H_
#define GPS_H_

#include <SoftwareSerial.h>  // software uart for GPS plerup/espsoftwareserial
#include "minmea.h"
#include "led.h"

class NMEAGPS {
public:
  NMEAGPS() {}
  ~NMEAGPS() {}

  bool begin(uint32_t baud, int8_t rxPin, int8_t txPin);
  void update();

  int getFixQuality()  { return (fixQuality); }
  float getCourse()    { return (course); }
  float getLatitude()  { return (latitude); }
  float getLongitude() { return (longitude); }
  float getSpeed()     { return (speed); }

  void getTime(struct tm* tm)  { memcpy(tm, &localtm, sizeof(struct tm)); }

  void getDataRate(enum minmea_sentence_id sid);
  
  void setDebug(bool on);

  void testFunc();

private:
  EspSoftwareSerial::UART gpsPort;
  LEDS leds;

  bool parsing;
  bool noSig;
  char sentence[MINMEA_MAX_SENTENCE_LENGTH + 1];
  uint8_t parsInx;
  uint32_t lastMsgOffset;

  bool dataValid;
  int fixQuality;

  float course;
  float longitude;
  float latitude;
  float speed;

  struct tm localtm;

  void parseSentence();
};

#endif // GPS_H_