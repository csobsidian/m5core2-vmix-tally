/*
 * 
 * Config.h - Library for handling persistent configuration variables.
 * Created by Cheyenne Meyer, 2021-10-28.
 * Released into the public domain.
 * 
 */
#ifndef Config_h
#define Config_h


// Includes
#include <M5Core2.h>
#include <IniFile.h>
#include <Preferences.h>


// Defines
#define CONFIG_BUFFER_LEN 80
#define CFG_INI_FILENAME "/vmix.ini"
#define CFG_PREFS_NAMESPACE "vmix"
#define CFG_PREFS_APP_VALUE "tally"


// Constants



class Config
{
  public:
    Config();
    void begin();
    bool getValue(const char *section, const char *key, char val[],    size_t maxLength, const char *defaultValue);
    bool getValue(const char *section, const char *key, uint16_t &val,                      uint16_t defaultValue);
    bool getValue(const char *section, const char *key, bool &val,                              bool defaultValue);

  private:

    const size_t _bufferLen = CONFIG_BUFFER_LEN;
    char _buffer[CONFIG_BUFFER_LEN];
    bool _sdOk;
    IniFile _file;
    Preferences _prefs;
};

extern Config cfg;

#endif
