#include "ledAddr.h"


#define LED_PIN 4
#define LED_CNT 2
#define LED_BRIGHT 80

static CRGB aleds[LED_CNT];

static uint32_t l1Offset;
static uint8_t l1State = 0;
static uint8_t r1 = 255;
static uint8_t g1 = 0;
static uint8_t b1 = 0;

static uint32_t l2Offset;
static uint8_t l2State = 0;
static uint8_t r2 = 255;
static uint8_t g2 = 0;
static uint8_t b2 = 0;

void setupAddr(void)
{
  FASTLED_MIN_SETUP(LED_PIN, aleds, LED_CNT);
  FastLED_min<LED_PIN>.setBrightness(LED_BRIGHT);

  l1Offset = l2Offset = millis();
}

void updateAddr(void)
{
  if ((millis() - l1Offset) > 100)
  {
    l1Offset = millis();
    switch(l1State)
    {
      // red down, green up
      case 0:
      {
        if (aleds[0].r == 0)
        {
          l1State = 1;
          aleds[0].g = 255;
        }
        else
        {
          --aleds[0].r;
          ++aleds[0].g;
        }
      }  break;

      // green down, blue up
      case 1:
      {
        if (aleds[0].g == 0)
        {
          l1State = 2;
          aleds[0].b = 255;
        }
        else
        {
          --aleds[0].g;
          ++aleds[0].b;
        }
      }  break;

      // blue down, blue red
      case 2:
      {
        if (aleds[0].b == 0)
        {
          l1State = 0;
          aleds[0].r = 255;
        }
        else
        {
          --aleds[0].b;
          ++aleds[0].r;
        }
      }  break;
    }
  }

  if ((millis() - l2Offset > 1000))
  {
    l2Offset = millis();
    switch(l2State)
    {
      case 0:
      {
        aleds[1].r = 255;
        aleds[1].g = 0;
        aleds[1].b = 0;
        l2State = 1;
      }  break;

      case 1:
      {
        aleds[1].r = 0;
        aleds[1].g = 255;
        aleds[1].b = 0;
        l2State = 2;
      }  break;

      case 2:
      {
        aleds[1].r = 0;
        aleds[1].g = 0;
        aleds[1].b = 255;
        l2State = 0;
      }  break;
    }
  }

}