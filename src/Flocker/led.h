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
  enum LED_id_t
  {
    LED_GPS,
    LED_COMMS
  };

  enum LED_color_t
  {
    CLR_RED = 0,
    CLR_GRN,
    CLR_BLU,
    CLR_MAX
  };

  LEDS() {}
  ~LEDS() {}

  bool begin(uint8_t pin, uint8_t bright);
  void update();

  void cycleRed(LED_id_t id, uint32_t base, uint32_t on);
  void cycleBlu(LED_id_t id, uint32_t base, uint32_t on);
  void cycleGrn(LED_id_t id, uint32_t base, uint32_t on);
  void cycle(LED_id_t id, LEDS::LED_color_t c, uint32_t base, uint32_t on);

  void steadyRed(LED_id_t id, uint8_t level);
  void steadyGrn(LED_id_t id, uint8_t level);
  void steadyBlu(LED_id_t id, uint8_t level);
  void steady(LED_id_t id, LEDS::LED_color_t c, uint8_t level);

  void stopRed(LED_id_t id);
  void stopGrn(LED_id_t id);
  void stopBlu(LED_id_t id);
  void stop(LED_id_t id, LEDS::LED_color_t c);

  void pulseRed(LED_id_t id, uint32_t duration);
  void pulseGrn(LED_id_t id, uint32_t duration);
  void pulseBlu(LED_id_t id, uint32_t duration);
  void pulse(LED_id_t id, LEDS::LED_color_t c, uint32_t duration);

  // do the "alert" pulse
  void alertRed(LED_id_t id);
  void alertGrn(LED_id_t id);
  void alertBlu(LED_id_t id);
  void alert(LED_id_t id, LEDS::LED_color_t c);

  // Is the LED is actually lit?
  bool isRedOn(LED_id_t id);
  bool isGrnOn(LED_id_t id);
  bool isBluOn(LED_id_t id);
  bool isLEDOn(LED_id_t id, LEDS::LED_color_t);

  // Is the LED active? (e.g., cycling, but not necessarily lit)
  bool isRedActive(LED_id_t id)                      { return(isLEDActive(id, LEDS::CLR_RED)); }
  bool isGrnActive(LED_id_t id)                      { return(isLEDActive(id, LEDS::CLR_GRN)); }
  bool isBluActive(LED_id_t id)                      { return(isLEDActive(id, LEDS::CLR_BLU)); }
  bool isLEDActive(LED_id_t id, LEDS::LED_color_t c) { return(leds[id][c].mode != LED_MODE_OFF); }

private:

  enum LED_mode_t
  {
    LED_MODE_OFF = 0,
    LED_MODE_STEADY,
    LED_MODE_PULSE,
    LED_MODE_ALERT,
    LED_MODE_CYCLE_ON,
    LED_MODE_CYCLE_OFF
  };

  struct led
  {
    uint8_t level;
    LEDS::LED_mode_t mode;
    uint32_t pulseTime;
    uint8_t alertTime;
    uint32_t offset;
    uint32_t cycleTime;
    uint32_t cycleOnTime;
  };

  struct led leds[2][CLR_MAX];
  uint8_t maxBright;
  uint8_t cfgListenerID;
};

#endif // LED_H_