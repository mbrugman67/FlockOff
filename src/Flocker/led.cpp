#include <LiteLED.h>

#include "led.h"
#include "flockCfg.h"
#include "globals.h"

static const crgb_t L_RED = 0xff0000;
static const crgb_t L_GREEN = 0x00ff00;
static const crgb_t L_BLUE = 0x0000ff;
static const crgb_t L_WHITE = 0xe0e0e0;

static LiteLED aLEDS(LED_STRIP_WS2812_RGB, 0);

/*******************************************
* begin()
********************************************
* set up this LED's pins, set mode to off
* for each LED color
*******************************************/
bool LEDS::begin(uint8_t pin, uint8_t bright)
{
  maxBright = bright;

  esp_err_t ledRet = aLEDS.begin(pin, 2);
  aLEDS.fill(0, 1);

  leds[LEDS::LED_GPS][LEDS::CLR_RED].mode = LEDS::LED_MODE_OFF;
  leds[LEDS::LED_GPS][LEDS::CLR_GRN].mode = LEDS::LED_MODE_OFF;
  leds[LEDS::LED_GPS][LEDS::CLR_BLU].mode = LEDS::LED_MODE_OFF;

  leds[LEDS::LED_COMMS][LEDS::CLR_RED].mode = LEDS::LED_MODE_OFF;
  leds[LEDS::LED_COMMS][LEDS::CLR_GRN].mode = LEDS::LED_MODE_OFF;
  leds[LEDS::LED_COMMS][LEDS::CLR_BLU].mode = LEDS::LED_MODE_OFF;

  for (uint8_t id = 0; id < 2; ++id)
  {
    for (uint8_t ii = 0; ii < LEDS::CLR_MAX; ++ii)
    {
      leds[id][ii].level =
      leds[id][ii].cycleOnTime =
      leds[id][ii].cycleTime =
      leds[id][ii].alertTime =
      leds[id][ii].pulseTime = 0;
      leds[id][ii].offset = millis();
    }
  }

  if (flockCfg.registerListener(cfgListenerID))
  {
    maxBright = flockCfg.getLEDBrightness();
  }

  return (ledRet == ESP_OK);
}

/*******************************************
* update()
********************************************
* call this periodically (at least once per
* millisecond) to process timing of all 3
* LEDs
*******************************************/
void LEDS::update()
{
  static uint32_t cfgWatch = millis();

  for (uint8_t id = 0; id < 2; ++id)
  {
    for (uint8_t ii = 0; ii < LEDS::CLR_MAX; ++ii)
    {
      led* thisLED = &leds[id][ii];  // get a pointer for simplicity
      uint32_t elapsed = millis() - thisLED->offset;

      switch (thisLED->mode)
      {
        case LEDS::LED_MODE_STEADY:
        {
          break;
        }
        case LEDS::LED_MODE_PULSE:
        {
          if (elapsed >= thisLED->pulseTime)
          {
            thisLED->level = 0;
            thisLED->mode = LEDS::LED_MODE_OFF;
          }
        }  break;

        case LEDS::LED_MODE_ALERT:
        {
          if (elapsed >= 2)
          {
            --thisLED->level;
            thisLED->offset = millis();

            if (thisLED->level == 0)
            {
              thisLED->level = maxBright;
            }
          }
        }  break;

        case LEDS::LED_MODE_CYCLE_OFF:
        {
          if (elapsed >= thisLED->cycleTime)
          {
            thisLED->level = maxBright;
            thisLED->mode = LED_MODE_CYCLE_ON;
            thisLED->offset = millis();
          }
        }  break;

        case LEDS::LED_MODE_CYCLE_ON:
        {
          if (elapsed >= thisLED->cycleOnTime)
          {
            thisLED->level = 0;
            thisLED->mode = LED_MODE_CYCLE_OFF;
            thisLED->offset = millis();
          }
        }  break;

        case LEDS::LED_MODE_OFF:
        {
          thisLED->level = 0;
        }  break;

        default:
        {
          // nothing to see here, move along
        }
      }
    }

    if ((millis() - cfgWatch >= 100))
    {
      cfgWatch = millis();

      if (flockCfg.newCfgAvailable(cfgListenerID))
      {
        maxBright = flockCfg.getLEDBrightness();
      }
    }
  }

  crgb_t val = 0;
  val =  (((uint32_t)leds[LED_GPS][CLR_RED].level << 16) & 0x00ff0000);
  val |= (((uint32_t)leds[LED_GPS][CLR_GRN].level <<  8) & 0x0000ff00);
  val |= (((uint32_t)leds[LED_GPS][CLR_BLU].level      ) & 0x000000ff);
  aLEDS.setPixel(LED_GPS, val);

  val = 0;
  val =  (((uint32_t)leds[LED_COMMS][CLR_RED].level << 16) & 0x00ff0000);
  val |= (((uint32_t)leds[LED_COMMS][CLR_GRN].level <<  8) & 0x0000ff00);
  val |= (((uint32_t)leds[LED_COMMS][CLR_BLU].level      ) & 0x000000ff);
  aLEDS.setPixel(LED_COMMS, val);

  aLEDS.show();
}

/*******************************************
* cycleXXX()
********************************************
* start the specified LED flashing with a
* given cycle time and on time (for variable
* duty cycle
*******************************************/
void LEDS::cycleRed(LED_id_t id, uint32_t base, uint32_t on)  { this->cycle(id, LEDS::CLR_RED, base, on); }
void LEDS::cycleGrn(LED_id_t id, uint32_t base, uint32_t on)  { this->cycle(id, LEDS::CLR_GRN, base, on); }
void LEDS::cycleBlu(LED_id_t id, uint32_t base, uint32_t on)  { this->cycle(id, LEDS::CLR_BLU, base, on); }
void LEDS::cycle(LED_id_t id, LEDS::LED_color_t c, uint32_t base, uint32_t on)
{
    if (leds[id][c].mode != LEDS::LED_MODE_CYCLE_ON && leds[id][c].mode != LEDS::LED_MODE_CYCLE_OFF)
    {
        leds[id][c].cycleTime = base;
        leds[id][c].cycleOnTime = on;
        leds[id][c].offset = millis();
        leds[id][c].mode = LEDS::LED_MODE_CYCLE_ON;
        leds[id][c].level = maxBright;
    }
}

/*******************************************
* steadyXXX()
********************************************
* continusouly light the specified LED
*******************************************/
void LEDS::steadyRed(LED_id_t id, uint8_t level)  { this->steady(id, LEDS::CLR_RED, level); }
void LEDS::steadyGrn(LED_id_t id, uint8_t level)  { this->steady(id, LEDS::CLR_GRN, level); }
void LEDS::steadyBlu(LED_id_t id, uint8_t level)  { this->steady(id, LEDS::CLR_BLU, level); }
void LEDS::steady(LED_id_t id, LEDS::LED_color_t c, uint8_t level)
{
  leds[id][c].level = level;
  leds[id][c].mode = LEDS::LED_MODE_STEADY;
}


/*******************************************
* stopXXX()
********************************************
* immediately turn off an LED.  Applies to
* both cycling (flashing) as well as single
* shot pulse
*******************************************/
void LEDS::stopRed(LED_id_t id)  { this->stop(id, LEDS::CLR_RED); }
void LEDS::stopGrn(LED_id_t id)  { this->stop(id, LEDS::CLR_GRN); }
void LEDS::stopBlu(LED_id_t id)  { this->stop(id, LEDS::CLR_BLU); }
void LEDS::stop(LED_id_t id, LEDS::LED_color_t c)
{
  leds[id][c].mode = LEDS::LED_MODE_OFF;
  leds[id][c].level = 0;
}

/*******************************************
* pulseXXX()
********************************************
* pulse (single shot) an LED for the specified
* time
*******************************************/
void LEDS::pulseRed(LED_id_t id, uint32_t duration)  { this->pulse(id, LEDS::CLR_RED, duration); }
void LEDS::pulseGrn(LED_id_t id, uint32_t duration)  { this->pulse(id, LEDS::CLR_GRN, duration); }
void LEDS::pulseBlu(LED_id_t id, uint32_t duration)  { this->pulse(id, LEDS::CLR_BLU, duration); }
void LEDS::pulse(LED_id_t id, LEDS::LED_color_t c, uint32_t duration)
{
  leds[id][c].offset = millis();
  leds[id][c].pulseTime = duration;
  leds[id][c].mode = LEDS::LED_MODE_PULSE;
  leds[id][c].level = maxBright;
}



void LEDS::alertRed(LED_id_t id)  { this->alert(id, LEDS::CLR_RED); }
void LEDS::alertGrn(LED_id_t id)  { this->alert(id, LEDS::CLR_GRN); }
void LEDS::alertBlu(LED_id_t id)  { this->alert(id, LEDS::CLR_BLU); }
void LEDS::alert(LED_id_t id, LEDS::LED_color_t c)
{
    if (leds[id][c].mode != LEDS::LED_MODE_ALERT)
    {
        leds[id][c].mode = LEDS::LED_MODE_ALERT;
        leds[id][c].level = maxBright;
    }
}



/*******************************************
* isXXXOn()
********************************************
* Returns true if LED is on (actually lit)
*******************************************/
bool LEDS::isRedOn(LED_id_t id)  { return (isLEDOn(id, LEDS::CLR_RED)); }
bool LEDS::isGrnOn(LED_id_t id)  { return (isLEDOn(id, LEDS::CLR_GRN)); }
bool LEDS::isBluOn(LED_id_t id)  { return (isLEDOn(id, LEDS::CLR_BLU)); }
bool LEDS::isLEDOn(LED_id_t id, LEDS::LED_color_t c)
{
  return (leds[id][c].level != 0);
}
