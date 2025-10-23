#include "Arduino.h"
#include "led.h"

#define LED_ON(x)    { pinMode(x, OUTPUT); digitalWrite(x, HIGH); }
#define LED_OFF(x)   { pinMode(x, OUTPUT); digitalWrite(x, LOW); }
#define IS_LED_ON(x) { digitalRead(x); }
/*******************************************
* begin()
********************************************
* set up this LED's pins, set mode to off
* for each LED color
*******************************************/
void LEDS::begin(uint8_t pin_r, uint8_t pin_g, uint8_t pin_b)
{
  leds[LEDS::CLR_RED].pin = pin_r;
  leds[LEDS::CLR_GRN].pin = pin_g;
  leds[LEDS::CLR_BLU].pin = pin_b;

  leds[LEDS::CLR_RED].mode = LEDS::LED_MODE_OFF;
  leds[LEDS::CLR_GRN].mode = LEDS::LED_MODE_OFF;
  leds[LEDS::CLR_BLU].mode = LEDS::LED_MODE_OFF;

  for (uint8_t ii = 0; ii < LEDS::CLR_MAX; ++ii)
  {
    leds[ii].cycleOnTime =
    leds[ii].cycleTime =
    leds[ii].pulseTime = 0;
    leds[ii].offset = millis();

    LED_OFF(leds[ii].pin);
  }
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
  for (uint8_t ii = 0; ii < LEDS::CLR_MAX; ++ii)
  {
    led* thisLED = &leds[ii];  // get a pointer for simplicity 
    uint32_t elapsed = millis() - thisLED->offset;

    switch (thisLED->mode)
    {
      case LEDS::LED_MODE_PULSE:
      {
        if (elapsed >= thisLED->pulseTime)
        {
          LED_OFF(thisLED->pin);
          thisLED->mode = LEDS::LED_MODE_OFF;
        }
      }  break;

      case LEDS::LED_MODE_CYCLE_OFF:
      {
        if (elapsed >= thisLED->cycleTime)
        {
          LED_ON(thisLED->pin);
          thisLED->mode = LED_MODE_CYCLE_ON;
          thisLED->offset = millis();
        }
      }  break;

      case LEDS::LED_MODE_CYCLE_ON:
      {
        if (elapsed >= thisLED->cycleOnTime)
        {
          LED_OFF(thisLED->pin);
          thisLED->mode = LED_MODE_CYCLE_OFF;
          thisLED->offset = millis();
        }
      }  break;

      case LEDS::LED_MODE_OFF:
      default:
      {
        // nothing to see here, move along
      }
    }
  }
}

/*******************************************
* cycleXXX()
********************************************
* start the specified LED flashing with a 
* given cycle time and on time (for variable
* duty cycle
*******************************************/
void LEDS::cycleRed(uint32_t base, uint32_t on)  { this->cycle(LEDS::CLR_RED, base, on); }
void LEDS::cycleGrn(uint32_t base, uint32_t on)  { this->cycle(LEDS::CLR_GRN, base, on); }
void LEDS::cycleBlu(uint32_t base, uint32_t on)  { this->cycle(LEDS::CLR_BLU, base, on); }
void LEDS::cycle(LEDS::LED_color_t c, uint32_t base, uint32_t on)
{
  leds[c].cycleTime = base;
  leds[c].cycleOnTime = on;
  leds[c].offset = millis();
  leds[c].mode = LEDS::LED_MODE_CYCLE_ON;
  LED_ON(leds[c].pin);
}

/*******************************************
* stopXXX()
********************************************
* immediately turn off an LED.  Applies to
* both cycling (flashing) as well as single
* shot pulse
*******************************************/
void LEDS::stopRed()  { this->stop(LEDS::CLR_RED); }
void LEDS::stopGrn()  { this->stop(LEDS::CLR_GRN); }
void LEDS::stopBlu()  { this->stop(LEDS::CLR_BLU); }
void LEDS::stop(LEDS::LED_color_t c)
{
  leds[c].mode = LEDS::LED_MODE_OFF;
  LED_OFF(leds[c].pin);
}

/*******************************************
* pulseXXX()
********************************************
* pulse (single shot) an LED for the specified
* time
*******************************************/
void LEDS::pulseRed(uint32_t duration)  { this->pulse(LEDS::CLR_RED, duration); }
void LEDS::pulseGrn(uint32_t duration)  { this->pulse(LEDS::CLR_GRN, duration); }
void LEDS::pulseBlu(uint32_t duration)  { this->pulse(LEDS::CLR_BLU, duration); }
void LEDS::pulse(LEDS::LED_color_t c, uint32_t duration)
{
  leds[c].offset = millis();
  leds[c].pulseTime = duration;
  leds[c].mode = LEDS::LED_MODE_PULSE;
  LED_ON(leds[c].pin);
}

/*******************************************
* isXXXOn()
********************************************
* Returns true if LED is on (actually lit)
*******************************************/
bool LEDS::isRedOn()  { return (isLEDOn(LEDS::CLR_RED)); }
bool LEDS::isGrnOn()  { return (isLEDOn(LEDS::CLR_GRN)); }
bool LEDS::isBluOn()  { return (isLEDOn(LEDS::CLR_BLU)); }
bool LEDS::isLEDOn(LEDS::LED_color_t c)
{
  return (bool)(IS_LED_ON(leds[c].pin));
}
