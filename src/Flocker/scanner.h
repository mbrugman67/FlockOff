#ifndef SCANNER_H_
#define SCANNER_H_

class SCANNER
{
public:
  SCANNER() {}
  ~SCANNER() {}

  bool begin();
  void survey(uint32_t interval, const char* fname, const char* notes);
  void scan(const char* logname);
  void update();

private:
  void startWiFi();
  void startBLE();
  void stopBLE();
  void stopScanning();
  void postSurveyActivities();

  bool scannerRunning;
  bool surveyStopTrigger;
  char surveyFileName[120];
  char surveyNotes[120];
  uint32_t channelTime;
  uint32_t channelTimeOffset;
  uint32_t btTimeOffset;
};

#endif //SCANNER_H_
