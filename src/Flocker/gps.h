#ifndef GPS_H_
#define GPS_H_

#include <SoftwareSerial.h>  // software uart for GPS plerup/espsoftwareserial
#include "minmea.h"

class NMEAGPS {
public:
  NMEAGPS() {}
  ~NMEAGPS() {}

  bool begin(uint32_t baud, int8_t rxPin, int8_t txPin);
  void update();

  int getFixQuality()       { return (fixQuality); }
  int getSatelliteCount()   { return (satelliteCount); }
  float getCourse()         { return (course); }
  float getLatitude()       { return (latitude); }
  float getLongitude()      { return (longitude); }
  float getSpeed()          { return (speed); }

  void getTime(struct tm* tm)  { memcpy(tm, &localtm, sizeof(struct tm)); }

  const char* getTimeLocationString();

private:
  EspSoftwareSerial::UART gpsPort;

  bool parsing;
  bool noSig;
  char sentence[MINMEA_MAX_SENTENCE_LENGTH + 1];
  uint8_t parsInx;
  uint32_t lastMsgOffset;

  bool once;
  bool loggedFix;
  bool firstRMC;
  bool firstGGA;
  bool firstGLL;
  bool timeIsSet;

  bool dataValid;
  int fixQuality;
  int satelliteCount;

  float course;
  float longitude;
  float latitude;
  float speed;

  struct tm localtm;

  void parseSentence();
};

#endif // GPS_H_