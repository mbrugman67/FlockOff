/***********************************************************
* Handle 3 individual (or one single 3-color) LED.
* 
* Initialize with the .begin() method, specifying the 
* Arduino pin number for the red, green, and blue elements.
*
* There are 3 general modes of operation for each color:
*  - Off: LED is off
*  - Cycling: LED is continually flashing at a fixed cycle
*             time (with the LED on for specified part 
*             of that time)
*  - Pulse: A single-shot blink for the specified time
*
* ALL TIMES ARE IN MILLISECONDS
***********************************************************/

#ifndef LED_H_
#define LED_H_

class LEDS
{
public:
  enum LED_color_t
  {
    CLR_RED = 0,
    CLR_GRN,
    CLR_BLU,
    CLR_MAX
  };

  LEDS() {}
  ~LEDS() {}

  void begin(uint8_t pin_r, uint8_t pin_g, uint8_t pin_b);
  void update();

  void cycleRed(uint32_t base, uint32_t on);
  void cycleBlu(uint32_t base, uint32_t on);
  void cycleGrn(uint32_t base, uint32_t on);
  void cycle(LEDS::LED_color_t c, uint32_t base, uint32_t on);


  void stopRed();
  void stopGrn();
  void stopBlu();
  void stop(LEDS::LED_color_t c);

  void pulseRed(uint32_t duration);
  void pulseGrn(uint32_t duration);
  void pulseBlu(uint32_t duration);
  void pulse(LEDS::LED_color_t c, uint32_t duration);

  // Is the LED is actually lit?
  bool isRedOn();
  bool isGrnOn();
  bool isBluOn();
  bool isLEDOn(LEDS::LED_color_t);

  // Is the LED active? (e.g., cycling, but not necessarily lit)
  bool isRedActive()                    { return(isLEDActive(LEDS::CLR_RED)); }
  bool isGrnActive()                    { return(isLEDActive(LEDS::CLR_GRN)); }
  bool isBluActive()                    { return(isLEDActive(LEDS::CLR_BLU)); }
  bool isLEDActive(LEDS::LED_color_t c) { return(leds[c].mode != LED_MODE_OFF); }

private:

  enum LED_mode_t
  {
    LED_MODE_OFF = 0,
    LED_MODE_PULSE,
    LED_MODE_CYCLE_ON,
    LED_MODE_CYCLE_OFF
  };

  struct led
  {
    uint8_t pin;
    LEDS::LED_mode_t mode;
    uint32_t pulseTime;
    uint32_t offset;
    uint32_t cycleTime;
    uint32_t cycleOnTime;
  };

  struct led leds[CLR_MAX];

};

#endif // LED_H_