#include "flockLog.h"
#include "flockFs.h"

#include "globals.h"

#define LOG_FILE_EXT "log"

#define LOG_BUFF_SIZE (8 * 1024)
#define LOG_LINE_SIZE 512

bool FLOGGER::begin(uint32_t interval, const char* baseName, uint8_t fileCount)
{
  buf = (char*)ps_malloc(LOG_BUFF_SIZE);
  if (!buf)
  {
    Serial.printf(CLI_BOLD_RED "\r\n\r\nFLOGGER::begin() - failed to allocate log buffer!\r\\r\n" CLI_RESET);
    return (false);
  }
  memset(buf, 0, LOG_BUFF_SIZE);

  line = (char*)ps_malloc(LOG_LINE_SIZE);
  if (!line)
  {
    Serial.printf(CLI_BOLD_RED "\r\n\r\nFLOGGER::begin() - failed to allocate line buffer!\r\\r\n" CLI_RESET);
    free(buf);
    return (false);
  }

  snprintf(this->fname, 63, "%s.%s", baseName, LOG_FILE_EXT);

  if (flockCfg.getDebugEnabledState())
  {
    flockfs.rollFiles(baseName, LOG_FILE_EXT, fileCount);
    this->addLogLine("LOGGER", "Started logger\r\n");
  }

  updateOffset = millis();
  updateInterval = interval;

  return (true);
}

void FLOGGER::close()
{
    if (buf)
    {
        this->addLogLine("LOG", "Closing log!\r\n");
        this->flushNow();
        free (buf);
    }
}

void FLOGGER::addLogLine(const char* src, const char* fmt, ...)
{
  char tstring[64] = {0};
  char fullFormatString[LOG_LINE_SIZE];

  time_t t = time(NULL);
  tm *tmp;
  tmp = localtime(&t);
  strftime(tstring, 63, "%D %T", tmp);

  // leader of log line    [12345678] 12/9/2025 17:57:24 SOURCE::
  snprintf(fullFormatString, LOG_LINE_SIZE - 1, "[%08d] %s %s::%s", millis(), tstring, src, fmt);

  va_list args;
  va_start(args, fmt);
  vsnprintf(line, LOG_LINE_SIZE - 1, fullFormatString, args);
  va_end(args);

  strncat(buf, line, (LOG_BUFF_SIZE - strlen(buf)));
}

void FLOGGER::update()
{
  if ((millis() - this->updateOffset) > this->updateInterval)
  {
    this->updateOffset = millis();

    if (flockCfg.getDebugEnabledState())
    {
      if (strlen(buf))
      {
        ssize_t appended = flockfs.appendFile(this->fname, (uint8_t*)buf, strlen(buf));
        buf[0] = '\0';
      }
    }
  }
}

void FLOGGER::flushNow()
{
  this->updateOffset += (2 * this->updateInterval);
  this->update();
}
