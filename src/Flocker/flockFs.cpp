/********************************************************************
 * flockFS.cpp
 ********************************************************************
 * Wrapper class around the ESP32S3 standard library file functions
 *******************************************************************/
#include "flockFs.h"

#include "globals.h"

/********************************************
* Instantiate the flash filesystem.  If the
* partitioning was changed, this will most
* likely fail.
********************************************/
bool MBFS::begin()
{
  if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED))
  {
    Serial.printf("Could not init file system\r\n");
    return (false);
  }

  return (true);
}

/*********************************************
* Format the filesystem.
**********************************************/
void MBFS::format()
{
    LittleFS.format();
}

/*********************************************
* list files in the filesystem.  Filenames are
* returned in a std::vector
**********************************************/
size_t MBFS::list(std::vector<std::string>& files)
{
    // all files are in the root directory; i.e. flat
    File root = LittleFS.open("/");
    File file = root.openNextFile();

    files.clear();

    while(file)
    {
        files.push_back(std::string(file.name()));
        file = root.openNextFile();
    }

    return (files.size());
}

bool MBFS::fileExists(const char* path)
{
    char fpath[65] = {0};
    snprintf(fpath, 64, "/%s", path);

    // try to open the file for reading, do not create if it doesnt exist
    File file = LittleFS.open(fpath, "r", false);

    if (file)
    {
        file.close();
        return (true);
    }
    return (false);
}

void MBFS::getInfo(size_t* cap, size_t* used)
{
    *cap = LittleFS.totalBytes();
    *used = LittleFS.usedBytes();
}

ssize_t MBFS::readFile(const char* path, uint8_t* buff, size_t len)
{
    char fpath[65] = {0};
    snprintf(fpath, 64, "/%s", path);
    File file = LittleFS.open(fpath);

    if(!file || file.isDirectory())
    {
        return (-1);
    }

    ssize_t read = file.read(buff, len);

    file.close();
    return (read);
}

ssize_t MBFS::writeFile(const char* path, const uint8_t* buff, size_t len)
{
    char fpath[65] = {0};
    snprintf(fpath, 64, "/%s", path);
    File file = LittleFS.open(fpath, FILE_WRITE);

    if(!file || file.isDirectory())
    {
        Serial.println("- failed to open file for writing");
        return (-1);
    }

    ssize_t written = file.write(buff, len);

    file.close();
    return (written);
}

ssize_t MBFS::appendFile(const char* path, const uint8_t* buff, size_t len)
{
    char fpath[65] = {0};
    snprintf(fpath, 64, "/%s", path);

    File file = LittleFS.open(fpath, FILE_APPEND);

    if(!file || file.isDirectory())
    {
        return (-1);
    }

    ssize_t written = file.write(buff, len);

    file.close();
    return (written);
}

ssize_t MBFS::getFileSize(const char* path)
{
    char fpath[65] = {0};
    snprintf(fpath, 64, "/%s", path);
    File file = LittleFS.open(fpath);

    if(!file || file.isDirectory())
    {
        return (-1);
    }

    ssize_t ret = file.size();

    file.close();
    return (ret);
}

bool MBFS::renameFile(const char* src, const char* dst)
{
    char spath[65] = {0};
    char dpath[65] = {0};

    snprintf(spath, 64, "/%s", src);
    snprintf(dpath, 64, "/%s", dst);

    if (LittleFS.rename(spath, dpath))
    {
        return (true);
    }
    else
    {
        return (false);
    }
}


bool MBFS::copyFile(const char* src, const char* dst)
{
    bool ret = false;
    char spath[65] = {0};
    char dpath[65] = {0};

    snprintf(spath, 64, "/%s", src);
    snprintf(dpath, 64, "/%s", dst);

    ssize_t copyLen = this->getFileSize(src);
    if (copyLen <= 0)
    {
        return (ret);
    }

    uint8_t* buf = (uint8_t*)ps_malloc(copyLen);
    if (buf)
    {
        ssize_t read = this->readFile(src, buf, copyLen);
        if (read != copyLen)
        {
            Serial.printf("copyFile() error - bad read\r\n");
        }
        else
        {
            ssize_t written = this->writeFile(dst, buf, copyLen);
            if (written != copyLen)
            {
                Serial.printf("copyFile() error - bad write\r\n");
            }
            else
            {
                ret = true;
            }
        }
        free (buf);
    }
    else
    {
        Serial.printf("copyFile() error - unable to allocate %d bytes for copy\r\n", copyLen);
    }

    return (ret);
}

bool MBFS::deleteFile(const char* path)
{
    char fpath[65] = {0};
    snprintf(fpath, 64, "/%s", path);

    if(LittleFS.remove(fpath))
    {
        return (true);
    }
    else
    {
        return (false);
    }
}

/****************************************************
* File roller.  Pass the base filename, extension,
* and max count.
*
* For example, if called as ->rollFiles("survey", "log", 5)
* the files would roll:
*    survey.5.log would get deleted (if it exists)
*    survey.4.log would get renamed survey.5.log (if it exists)
*    survey.3.log would get renamed survey.4.log (if it exists)
*    survey.2.log would get renamed survey.3.log (if it exists)
*    survey.1.log would get renamed survey.2.log (if it exists)
*    survey.log would get renamed survey.1.log (if it exists)
****************************************************/
bool MBFS::rollFiles(const char* basePath, const char* ext, uint8_t count)
{
    char* newName = (char*)ps_malloc(128);
    if (!newName)
    {
        return (false);
    }

    char* oldName = (char*)ps_malloc(128);
    if (!oldName)
    {
        free(newName);
        return (false);
    }

    // remove the oldest file (if it exists)
    snprintf(newName, 127, "%s.%d.%s", basePath, count--, ext);
    this->deleteFile(newName);

    while (count > 0)
    {
        snprintf(newName, 127, "%s.%d.%s", basePath, count--, ext);
        this->deleteFile(newName);

        if (!count)
        {
            break;
        }

        snprintf(oldName, 127, "%s.%d.%s", basePath, count, ext);
        this->renameFile(oldName, newName);
    }

    snprintf(oldName, 127, "%s.%s", basePath, ext);
    this->renameFile(oldName, newName);

    free(oldName);
    free(newName);

    return (true);
}
