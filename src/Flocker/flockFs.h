#ifndef MBFS_H_
#define MBFS_H_

#include <vector>

#include "LittleFS.h"
#include "alloc.h"

class MBFS
{
public:
  MBFS() {}
  ~MBFS() {}

  bool begin();

  void getInfo(size_t* cap, size_t* used);
  void format();

  size_t list(std::vector<flk::string>& files);
  ssize_t readFile(const char* path, uint8_t* buff, size_t len);
  ssize_t writeFile(const char* path, const uint8_t* buff, size_t len);
  ssize_t appendFile(const char* path, const uint8_t* buff, size_t len);
  ssize_t getFileSize(const char* path);
  bool renameFile(const char* src, const char* dst);
  bool copyFile(const char* src, const char* dst);
  bool deleteFile(const char* path);
  bool fileExists(const char* path);
  bool rollFiles(const char* basePath, const char* ext, uint8_t count);

  const char* lastFileWrite(const char* path);

private:
};


#endif // MBFS_H_
