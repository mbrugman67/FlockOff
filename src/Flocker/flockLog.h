#ifndef FLOCK_LOG_H_
#define FLOCK_LOG_H_

#include <stdarg.h>
#include <stdint.h>

class FLOGGER
{
public:
  FLOGGER() {}
  ~FLOGGER() {}

  bool begin(uint32_t interval, const char* baseName, uint8_t fileCount);
  void addLogLine(const char* src, const char* fmt, ...);
  void update();
  void flushNow();
  void close();

private:
  char* buf;
  char* line;
  char fname[64];

  uint32_t updateOffset;
  uint32_t updateInterval;
};

#endif // FLOCK_LOG_H_
