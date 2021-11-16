/*
 * 
 * Config.cpp - Library for handling persistent configuration variables.
 * Created by Cheyenne Meyer, 2021-10-28.
 * Released into the public domain.
 * 
 * Active config from SD card if present (m5stack.cfg).
 * Read EEPROM.
 * Active config from EEPROM if SD config issue (no SD card, no file, bad parse).
 * Active config from defaults if EEPROM issue (bad parse).
 * Write config to EEPROM if active config does not match.
 *
 */
#include <Preferences.h>
#include <SD.h>
#include <IniFile.h>
#include "Config.h"

    
Config::Config() : _file(CFG_INI_FILENAME) {
}

 
void Config::begin() {

  if (!SD.begin()) {
    //SD card error
    M5.lcd.println("Error reading SD card.");
  } else if (!_file.open()) {
    M5.lcd.println("Error opening file.");
    M5.lcd.println(CFG_INI_FILENAME);
    delay(1000);
  } else if (!_file.validate(_buffer, _bufferLen)) {
    M5.lcd.println("Ini file invalid.");
    M5.lcd.println(CFG_INI_FILENAME);
    delay(1000);
  } else {
    _sdOk = true;
  }

  _prefs.begin(CFG_PREFS_NAMESPACE, false);
  String app = _prefs.getString("app", "");
  if (!app.equals(CFG_PREFS_APP_VALUE)) {
    _prefs.clear();
    _prefs.putString("app", CFG_PREFS_APP_VALUE);
  }
}


bool Config::getValue(const char *section, const char *key, char val[], size_t maxLength, const char *defaultValue) {
  bool _iniRead = false;
  char iniVal[maxLength];

  // Get value from ini file
  if (_sdOk) {
    if (_file.getValue(section, key, _buffer, _bufferLen)) {
      strcpy(iniVal, _buffer);
      _iniRead = true;
    }
  }
  
  // Get value from NVRAM
  bool _prefRead = false;
  char prefVal[maxLength];
  char prefKey[strlen(section) + strlen(key) + 2]; // "<section>-<key>" + null terminator
  strcpy(prefKey, section);
  strcat(prefKey, "-");
  strcat(prefKey, key);
  if (_prefs.isKey(prefKey)) {
    int prefLen = _prefs.getString(prefKey, prefVal, sizeof(prefVal));
    _prefRead = true;
  }

  // If iniRead = true, use it. Otherwise, if prefRead = true, use it. Otherwise, use default value.
  if (_iniRead) {
    strcpy(val, iniVal);
    M5.lcd.printf("Cfg [ini] -> '%s' %s\n", prefKey, val);
  } else if (_prefRead) {
    strcpy(val, prefVal);
    M5.lcd.printf("Cfg [prf] -> '%s' %s\n", prefKey, val);
  } else {
    strcpy(val, defaultValue);
    M5.lcd.printf("Cfg [def] -> '%s' %s\n", prefKey, val);
  }

  // If chosen value != prefVal, write to NVRAM.
  if (!_prefRead || (strcmp(val, prefVal) != 0)) {
    _prefs.putString(prefKey, val);
    M5.lcd.printf("Cfg [prf] <- '%s' %s\n", prefKey, val);
  }
  
  return true;
}


bool Config::getValue(const char *section, const char *key, uint16_t &val, uint16_t defaultValue) {
  bool _iniRead = false;
  uint16_t iniVal;

  // Get value from ini file
  if (_sdOk) {
    if (_file.getValue(section, key, _buffer, _bufferLen, iniVal)) {
      _iniRead = true;
    }
  }
  
  // Get value from NVRAM
  bool _prefRead = false;
  uint16_t prefVal;
  char prefKey[strlen(section) + strlen(key) + 2]; // "<section>-<key>" + null terminator
  strcpy(prefKey, section);
  strcat(prefKey, "-");
  strcat(prefKey, key);
  if (_prefs.isKey(prefKey)) {
    prefVal = _prefs.getUShort(prefKey, 0);
    _prefRead = true;
  }

  // If iniRead = true, use it. Otherwise, if prefRead = true, use it. Otherwise, use default value.
  if (_iniRead) {
    val = iniVal;
    M5.lcd.printf("Cfg [ini] -> '%s' %d\n", prefKey, val);
  } else if (_prefRead) {
    val = prefVal;
    M5.lcd.printf("Cfg [prf] -> '%s' %d\n", prefKey, val);
  } else {
    val = defaultValue;
    M5.lcd.printf("Cfg [def] -> '%s' %d\n", prefKey, val);
  }

  // If chosen value != prefVal, write to NVRAM.
  if ((!_prefRead) || (val != prefVal)) {
    _prefs.putUShort(prefKey, val);
    M5.lcd.printf("Cfg [prf] <- '%s' %d\n", prefKey, val);
  }
  
  return true;
}


bool Config::getValue(const char *section, const char *key, bool &val, bool defaultValue) {
  bool _iniRead = false;
  bool iniVal;

  // Get value from ini file
  if (_sdOk) {
    if (_file.getValue(section, key, _buffer, _bufferLen, iniVal)) {
      _iniRead = true;
    }
  }
  
  // Get value from NVRAM
  bool _prefRead = false;
  bool prefVal;
  char prefKey[strlen(section) + strlen(key) + 2]; // "<section>-<key>" + null terminator
  strcpy(prefKey, section);
  strcat(prefKey, "-");
  strcat(prefKey, key);
  if (_prefs.isKey(prefKey)) {
    prefVal = _prefs.getBool(prefKey, defaultValue);
    _prefRead = true;
  }

  // If iniRead = true, use it. Otherwise, if prefRead = true, use it. Otherwise, use default value.
  if (_iniRead) {
    val = iniVal;
    M5.lcd.printf("Cfg [ini] -> '%s' %s\n", prefKey, val ? "true" : "false");
  } else if (_prefRead) {
    val = prefVal;
    M5.lcd.printf("Cfg [prf] -> '%s' %s\n", prefKey, val ? "true" : "false");
  } else {
    val = defaultValue;
    M5.lcd.printf("Cfg [def] -> '%s' %s\n", prefKey, val ? "true" : "false");
  }

  // If chosen value != prefVal, write to NVRAM.
  if ((!_prefRead) || (val != prefVal)) {
    _prefs.putUShort(prefKey, val);
    M5.lcd.printf("Cfg [prf] <- '%s' %s\n", prefKey, val ? "true" : "false");
  }
  
  return true;
}



Config cfg;
