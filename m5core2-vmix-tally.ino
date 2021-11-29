// Includes
#include <M5Core2.h>
#include <WiFi.h>
#include "Config.h"
#include <limits.h>


// Defines
#define MAX_PASS_LEN 65
#define MAX_ADDR_LEN 16
#define MAX_TITLE_LEN 65
#define CFG_SSID_DEFAULT ""
#define CFG_PASS_DEFAULT ""
#define CFG_ADDR_DEFAULT ""
#define CFG_PORT_DEFAULT 8099
#define CFG_TALLY_INPUT_DEFAULT 2
#define CFG_TALLY_TITLE_DEFAULT true
#define CFG_DISPLAY_TIMEOUT_MS_DEFAULT 60000


// Enums
enum appStatus {
  NONE,
  INIT,
  WIFI_CONNECT,
  VMIX_CONNECT,
  VMIX_BAD_RESPONSE,
  TALLY_SAFE,
  TALLY_PREVIEW,
  TALLY_LIVE
};


// Constants
const char *vmixTallyCommand = "SUBSCRIBE TALLY\r\n";
const char *vmixTallyReply = "TALLY OK ";
const char *vmixXmlTextFmt = "XMLTEXT %s\r\n";
const char *vmixXmlTextTitleFmt = "vmix/inputs/input[%d]/@title";
const char *vmixXmlTextReply = "XMLTEXT OK ";
char vmixTitleCommand[60]; // more than enough for sprintf

// Globals
appStatus state;
appStatus lastState;
WiFiClient client;
bool subscribed = false;
char ssid[MAX_SSID_LEN + 1]; // max 32 char SSID + null char.
char pass[65]; // mac 64 char password + null char.
char addr[16]; // max 15 char v4 ip address + null char.
uint16_t port;
uint16_t inputNum;
bool showTitle;
char title[MAX_TITLE_LEN];
int pos;
uint16_t displayTimeoutMs;


void setup()
{
  // Init M5
  M5.begin();

  showStatus(INIT);
  delay(1000);

  // Get config vars
  cfg.begin();
  cfg.getValue("network",      "ssid", ssid, sizeof(ssid), CFG_SSID_DEFAULT);
  cfg.getValue("network",      "pass", pass, sizeof(pass), CFG_PASS_DEFAULT);
  cfg.getValue("vMix",         "addr", addr, sizeof(addr), CFG_ADDR_DEFAULT); 
  cfg.getValue("vMix",         "port", port,               CFG_PORT_DEFAULT);
  cfg.getValue("vMix",        "tally", inputNum,           CFG_TALLY_INPUT_DEFAULT);
  cfg.getValue("vMix",   "show-title", showTitle,          CFG_TALLY_TITLE_DEFAULT);
  cfg.getValue("vMix", "disp-timeout", displayTimeoutMs,   CFG_DISPLAY_TIMEOUT_MS_DEFAULT);

  pos = strlen(vmixTallyReply) + inputNum - 1;

  if (showTitle) {
    char temp[sizeof(vmixTitleCommand)];
    int num = sprintf(temp, vmixXmlTextTitleFmt, inputNum);
    num = sprintf(vmixTitleCommand, vmixXmlTextFmt, temp);
  }

  delay(3000);
  // Start WiFi
  WiFi.begin(ssid, pass);
  client.setTimeout(1);
}



void loop()
{
  if(WiFi.status() != WL_CONNECTED) {
    doWiFiConnect();
  }

  if(!client.connected()) {
    doClientConnect();
  } else {
    doClientComms();
  }
}

void doWiFiConnect() {
  unsigned long t1 = millis();  
  unsigned long t2 = t1;
  bool screenHidden = false;
  
  showStatus(WIFI_CONNECT);

  if (client.connected()) {
    client.stop();
    client.flush();
    subscribed = false;
  }

  int i = 0;
  while(WiFi.status() != WL_CONNECTED) {
    M5.update();
    if (screenHidden && TOUCH->ispressed()) {
      t1 = millis();
      t2 = t1;
      screenHidden = saveScreen(false);
    }
    
    if (i > 2) {
      M5.Lcd.fillRect(0, 96, 320, 16, BLACK);
      M5.Lcd.setCursor(12, 96);
      i = 0;
    }
    M5.Lcd.printf(".");
    i++;
    delay(500);

    t2 = millis();
    if ((deltaMs(t1, t2) > displayTimeoutMs) && (!screenHidden)) {
      screenHidden = saveScreen(true);
    }
  }

  if (screenHidden) {
    saveScreen(false);
  }
  
  M5.Lcd.fillRect(0, 96, 320, 16, BLACK);
  M5.Lcd.setCursor(12, 96);
  M5.Lcd.print("Connected!\n\n IP: ");
  M5.Lcd.print(WiFi.localIP());
  delay(3000);
}

void doClientConnect() {
  unsigned long t1 = millis();  
  unsigned long t2 = t1;
  bool screenHidden = false;

  showStatus(VMIX_CONNECT);
  
  subscribed = false;
  int i = 0;
  while (!client.connect(addr, port)) {
    M5.update();
    if (screenHidden && TOUCH->ispressed()) {
      t1 = millis();
      t2 = t1;
      screenHidden = saveScreen(false);
    }
    
    if (WiFi.status() != WL_CONNECTED) {
      return;
    }

    if (i > 2) {
      M5.Lcd.fillRect(0, 72, 320, 16, BLACK);
      M5.Lcd.setCursor(12, 72);
      i = 0;
    }
    
    M5.Lcd.printf(".");
    i++;
    // No delay. Arduino WiFiClient setTimeout is in seconds (1)
    //delay(500);

    t2 = millis();
    if ((deltaMs(t1, t2) > displayTimeoutMs) && (!screenHidden)) {
      screenHidden = saveScreen(true);
    }
  }

  if (screenHidden) {
    saveScreen(false);
  }
  
  M5.Lcd.fillRect(0, 96, 320, 16, BLACK);
  M5.Lcd.setCursor(12, 96);
  M5.Lcd.print("Connected!\n");
  delay(1000);
}


void doClientComms() {
  static unsigned long t1;
  
  if (!subscribed) {
    client.print(vmixTitleCommand);
    client.print(vmixTallyCommand);
    subscribed = true;
  }

  if (deltaMs(t1, millis()) > 5000) {
    client.print(vmixTitleCommand);
    t1 = millis();
  }


  while (client.available() > 0){
    String line = client.readStringUntil('\r\n');
    Serial.println(line);
    
    if (line.startsWith(vmixTallyReply)) {
      if (pos < line.length()) {
        switch (line.charAt(pos)) {
          case '1':
            showStatus(TALLY_LIVE);
            break;
          case '2':
            showStatus(TALLY_PREVIEW);
            break;
          case '0':
            showStatus(TALLY_SAFE);
            break;
          default:
            showStatus(VMIX_BAD_RESPONSE);
            m5.lcd.println(line);
            break;
        }
      }
    } else if (line.startsWith(vmixXmlTextReply)) {
      strncpy(title, line.substring(strlen(vmixXmlTextReply)).c_str(), sizeof(title));
      if (line.length() > sizeof(title) - 1) {
        title[64] = '\0';
      }

      t1 = millis();
    }
  }
}


void showStatus(appStatus s) {
  lastState = state;
  state = s;

  if (state == lastState) {
    return;
  }

  switch (s) {
    case NONE:
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(0,0);
      break;
    case INIT:
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.setCursor(0,0);
      M5.Lcd.setTextSize(1);
      M5.Lcd.printf("\nInitializing...\n\n");
      break;
    case WIFI_CONNECT:
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.setCursor(0,0);
      M5.Lcd.setTextSize(2);
      byte mac[6];
      WiFi.macAddress(mac);
      M5.Lcd.printf("\n Connecting to Wifi.\n\n  MAC: %02X:%02X:%02X:%02X:%02X:%02X\n SSID: %s\n\n ",
                      mac[5], mac[4], mac[3], mac[2], mac[1], mac[0], ssid);
      break;
    case VMIX_CONNECT:
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.setCursor(0,0);
      M5.Lcd.setTextSize(2);
      M5.Lcd.printf("\n Connecting to vMix:\n IP: %s\n\n ", addr);
      break;
    case VMIX_BAD_RESPONSE:
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.setCursor(0,0);
      M5.Lcd.setTextSize(2);
      M5.Lcd.printf("\n Error in vMix reply:\n ");
      break;
    case TALLY_SAFE:
      M5.Lcd.fillScreen(BLUE);
      M5.Lcd.setTextColor(BLACK);
      M5.Lcd.setTextSize(4);
      M5.Lcd.setCursor(112, 104);
      M5.Lcd.print("SAFE");
      if (showTitle) {
        M5.Lcd.setCursor(1, 1);
        M5.Lcd.setTextSize(3);
        M5.Lcd.print(title);
      }
      break;
    case TALLY_PREVIEW:
      M5.Lcd.fillScreen(GREEN);
      M5.Lcd.setTextColor(BLACK);
      M5.Lcd.setTextSize(4);
      M5.Lcd.setCursor(76, 104);
      M5.Lcd.printf("PREVIEW");
      if (showTitle) {
        M5.Lcd.setCursor(1, 1);
        M5.Lcd.setTextSize(3);
        M5.Lcd.print(title);
      }
      break;
    case TALLY_LIVE:
      M5.Lcd.fillScreen(RED);
      M5.Lcd.setTextColor(BLACK);
      M5.Lcd.setTextSize(4);
      M5.Lcd.setCursor(112, 104);
      M5.Lcd.printf("LIVE");
      if (showTitle) {
        M5.Lcd.setCursor(1, 1);
        M5.Lcd.setTextSize(3);
        M5.Lcd.print(title);
      }
      break;
    default:
      break;
  }
  
}

unsigned long deltaMs(unsigned long t1, unsigned long t2) {
  if (t1 <= t2) {
    return (t2 - t1);
  } else {
    return (ULONG_MAX - t1) + t2;
  }
}


bool saveScreen(bool save) {
  if (save) {
    // Turn off backlight and display
    M5.Lcd.startWrite();
    M5.Lcd.writecommand(ILI9341_DISPOFF);
    M5.Lcd.endWrite();
    M5.Lcd.setBrightness(0);
  } else {
    // Turn on backlight and display
    M5.Lcd.startWrite();
    M5.Lcd.writecommand(ILI9341_DISPON);
    M5.Lcd.endWrite();
    M5.Lcd.setBrightness(100);
  }

  return save;
}
