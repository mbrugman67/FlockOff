#include "Arduino.h"
#include "gps.h"
#include "led.h"
#include "flockLog.h"

#include "globals.h"

bool NMEAGPS::begin(uint32_t baud, int8_t rxPin, int8_t txPin)
{
  parsing = false;
  parsInx = 0;

  dataValid = false;
  fixQuality = 0;
  satelliteCount = 0;

  once = false;
  loggedFix = false;
  firstRMC = false;
  firstGGA = false;
  firstGLL = false;
  timeIsSet = false;

  course = 0.0;
  latitude = 0.0;
  longitude = 0.0;
  speed = 0.0;

  gpsPort.begin(baud, SWSERIAL_8N1, txPin, rxPin);

  lastMsgOffset = millis();

  return(gpsPort);
}

void NMEAGPS::parseSentence() 
{
  static uint32_t offset = millis();

  switch (minmea_sentence_id(sentence, false)) 
  {
    case MINMEA_SENTENCE_RMC:
    {
      struct minmea_sentence_rmc frame;
      if (minmea_parse_rmc(&frame, sentence)) 
      {
        this->latitude = minmea_tocoord(&frame.latitude);
        this->longitude = minmea_tocoord(&frame.longitude);
        this->speed = minmea_tofloat(&frame.speed);
        this->course = minmea_tofloat(&frame.course);

        minmea_getdatetime(&this->localtm, &frame.date, &frame.time);
        once = true;
        firstRMC = true;
      }
      else
      {
        flockLog.addLogLine("GPS", "Error parsing RMC sentence\r\n");
      }
    }  break;

    case MINMEA_SENTENCE_GGA: 
    {
      struct minmea_sentence_gga frame;
      if (minmea_parse_gga(&frame, sentence)) 
      {
        this->fixQuality = frame.fix_quality;
        this->satelliteCount = frame.satellites_tracked;
        once = true;
        firstGGA = true;
      }
      else
      {
        flockLog.addLogLine("GPS", "Error parsing GGA sentence\r\n");
      }
    }  break;

    case MINMEA_SENTENCE_GLL:
    {
      struct minmea_sentence_gll frame;
      if (minmea_parse_gll(&frame, sentence))
      {
        if (frame.status == MINMEA_GLL_STATUS_DATA_VALID)
        {
          dataValid = true;
        }
        else
        {
          dataValid = false;
        }
        once = true;
        firstGLL = true;
      }
      else
      {
        flockLog.addLogLine("GPS", "Error parsing GLL sentence\r\n");
      }
    }  break;

    case MINMEA_UNKNOWN:
    //case MINMEA_INVALID:
    case MINMEA_SENTENCE_TXT:
    {
      flockLog.addLogLine("GPS", "Error parsing sentence >%s<\r\n", sentence);
    }  break;
  }

  if (fixQuality == 0 && dataValid)
  {
    fixQuality = 1;
  }

  if (once)
  {
    if ((fixQuality > 0) || dataValid)
    {
      if ((millis() - offset) > 750)
      {
        offset = millis();
        flockLED.pulseGrn(LEDS::LED_GPS, 10);
        flockLED.stopBlu(LEDS::LED_GPS);
        flockLED.stopRed(LEDS::LED_GPS);

        if (!loggedFix)
        {
          flockLog.addLogLine("GPS", "Fix acquired\r\n");
          loggedFix = true;
        }
      }
    }
    else if (!flockLED.isBluActive(LEDS::LED_GPS))
    {
      flockLED.cycleBlu(LEDS::LED_GPS, 2500, 10);
      flockLED.cycleRed(LEDS::LED_GPS, 2500, 10);
      flockLog.addLogLine("GPS", "Fix lost\r\n");
      loggedFix = false;
    }
  } 
}

const char* NMEAGPS::getTimeLocationString()
{
  static char ret[128] = {'\0'};
  static char tim[32] = {'\0'};

  strftime(tim, 63, "%D %T", &localtm);
  snprintf(ret, 127, "%s %0.5f %0.5f", tim, latitude, longitude);
  return (ret);
}

void NMEAGPS::update()
{
  if (firstGGA && firstGLL && firstRMC && !timeIsSet)
  {
    setenv("TZ", "GMT0", 1);
    tzset();

    timeval setTime;
    setTime.tv_sec = mktime(&this->localtm);
    setTime.tv_usec = 0;
    settimeofday(&setTime, NULL);

    flockCfg.setTimeZone();
    timeIsSet = true;
    flockLog.addLogLine("GPS", "Initial clock set done\r\n");
  } 

  if (gpsPort.available())
  {
    char c = gpsPort.read();

    if (!parsing) 
    {
      if (c == '$') 
      {
        parsInx = 0;
        memset(sentence, 0x00, MINMEA_MAX_SENTENCE_LENGTH + 1);
        parsing = true;
      } // leading $
    } // not parsing

    if (parsing) 
    {
      if (parsInx >= MINMEA_MAX_SENTENCE_LENGTH)
      {
        parsing = false;
        return;
      }

      sentence[parsInx++] = c;

      if (c == '\n') 
      {
        lastMsgOffset = millis();
        this->parseSentence();
      } // final newline
    } // parsing == true
  }  // char available

  if ((millis() - lastMsgOffset) > 10000)
  {
    lastMsgOffset = millis();
    flockLED.pulseRed(LEDS::LED_GPS, 50);
    flockLED.stopBlu(LEDS::LED_GPS);
  }
}  // update()
