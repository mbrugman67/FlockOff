#ifndef SCANNER_H_
#define SCANNER_H_

class SCANNER
{
public:
  SCANNER() {}
  ~SCANNER() {}

  bool begin();
  void survey(uint32_t interval, bool doWiFi, bool doBT, const char* fname, bool doJson, const char* notes);
  void scan(const char* logname);

private:
  void startBLE();
  void stopBLE();

};

#endif //SCANNER_H_
