
/*
This code displays a rubik's cube to show the time of day.
Adafruit 128x128 TFT display
Processor needs to be fairly powerful. An Arduino ESP32 works well.

The pin configuration reflects the Wemos S2 mini using only the outer connectors.
*/

#include <WiFi.h>

#include <AnimatedGIF.h>

#include "Adafruit_GFX.h"
#include <Adafruit_ST7735.h>      // Hardware-specific library
#include <SdFat.h>                // SD card & FAT filesystem library
#include <Adafruit_SPIFlash.h>    // SPI / QSPI flash library
#include <Adafruit_ImageReader.h> // Image-reading functions

// Define the style of clock.  AM/PM or 24-hour
// Define only one of these.

#define CLOCK12
//#define CLOCK24

#define ENABLE_WIFI

#define ENABLE_RTC_PCF8523  // use external PCF8532 Real Time Clock
//#define ENABLE_RTC_ESP32     // use internal ESP32 RTC

// define this to set the real time clock to be build date
#define INITIALIZE_RTC

// Arduino will deep sleep after timeout then wake on touch
//#define ENABLE_SLEEP

#if defined(ARDUINO_ADAFRUIT_FEATHER_ESP32_V2) // Feather Huzzah32 V2 ESP32

// https://learn.adafruit.com/adafruit-esp32-feather-v2

  //#error Compiling for Adafruit Feather ESP32 V2

  #define FEATHER_V2 // Adafruit Feather ESP32-V2 product ??

  //#define I2C_SDA 22
  //#define I2C_SCL 20

  #define SD_CS      32 // SD card select pin
  #define TFT_CS     33 // TFT select pin
  #define TFT_RST    14 // TFT reset pin
  #define TFT_DC     15 // TFT Data/Command pin

#elif defined (ARDUINO_ADAFRUIT_FEATHER_ESP32S2) // Feather Huzzah32 S2

#define FEATHER_S2 // Adafruit Feather ESP32-S2 product 5000 or 5303

  //#error Compiling for Adafruit Feather S2

  #define I2C_SDA 3
  #define I2C_SCL 4

  #define SD_CS      6 // SD card select pin
  #define TFT_CS    12 // TFT select pin
  #define TFT_RST    5 // TFT reset pin
  #define TFT_DC     9 // TFT Data/Command pin

#elif defined(ARDUINO_LOLIN_S2_MINI) // WEMOS S2

//#error Compiling for WEMO S2

#define WEMOS_S2_MINI 

#define I2C_SDA 33
#define I2C_SCL 35

#define SD_CS     16 // SD card select pin
#define TFT_CS    39 // TFT select pin
#define TFT_RST    5 // TFT reset pin
#define TFT_DC     3 // TFT Data/Command pin
#define TFT_BACKLIGHT 37 // set low to blacken screen
#define TOUCH_PIN 12

#else

//#error unsupported board.  You will need to define a pin wiring selection for your board. Take a look at the first line in your compilation output.  prepend ARDUINO_ then paste that value but it needs to be upper case. E.g FQBN: esp32:esp32:lolin_s2_mini becomes #if defined(ARDUINO_LOLIN_S2_MINI)
/*
 * ESP32-C3 various dev board  : CS:  7, DC:  2, RST:  1, BL:  3, SCK:  4, MOSI:  6, MISO: nil
 * ESP32-S3 various dev board  : CS: 40, DC: 41, RST: 42, BL: 48, SCK: 36, MOSI: 35, MISO: nil
*/
  #define I2C_SDA 19
  #define I2C_SCL 21

#define SDCARD_SS_PIN 62

  #define SD_CS      10 // SD card select pin
  #define TFT_CS    40 // TFT select pin
  #define TFT_RST    42 // TFT reset pin
  #define TFT_DC     41 // TFT Data/Command pin
#define TFT_BACKLIGHT 2 // set low to blacken screen
#define TOUCH_PIN 12
#endif


#ifdef CLOCK12
#ifdef CLOCK24
#error Specify only 12-hour clock or 24-hour clock
#endif
#endif

#ifdef CLOCK12
#define GIF_FOLDER "/12hourclock-128x128/"
#else // CLOCK24
#define GIF_FOLDER "/24hourclock-128x128/"
#endif

#ifdef ENABLE_RTC_PCF8523
#ifdef ENABLE_RTC_ESP32
#error enable only PCF8523 external RTC or ESP32 internal RTC
#endif
#endif

//#ifdef ENABLE_RTC_ESP32
#include <ESP32Time.h>
#include <TimeLib.h>
//#endif
//#ifdef ENABLE_RTC_PCF8523
#include "RTClib.h"
//#endif

#ifdef ENABLE_SLEEP
RTC_DATA_ATTR int bootCount = 0; // testing
touch_pad_t touchPin;
#define THRESHOLD 400 //40

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  120        /* Time ESP32 will sleep for (in seconds) */

#define WAKE_TIME 1  // minutes of inactivity before next sleep
#endif

int countSinceSleep = 0;

#ifdef ENABLE_WIFI
// It is recommended to modify wifi.conf with your network credentials. wifi.conf is found in the root directory of the sd card.
// But you can choose to set a default network name and password if you
// prefer.  If so, then remove wifi.conf from the sdcard.
// Warning: never save this information in a public repo. Never ever commit credentials in git. History is preserved forever and hackers do know how to find it!

String wifiSsid     = "ssid";      // Your network name
String wifiPassword = "password";  // WARNING! do not accidentally  save password to GitHub
#endif // ENABLE_WIFI

#ifdef ENABLE_RTC_PCF8523
RTC_PCF8523 rtc8523;
bool validRtc8523 = false;
#endif //ENABLE_RTC_PCF8523

//char* ntpServer = "pool.ntp.org";
char* ntpServer = "time1.google.com";
const char* ntpServer1 = "pool.ntp.org";
//const char* ntpServer2 = "time.nist.gov";
const char* ntpServer2 = "time.apple.com";

long  gmtOffset_sec = 0; //-5*60*60; //0; //3600;
int   daylightOffset_sec = 0; //3600;
String timezoneStr = "GMT0"; // default timezone = London

#define DISPLAY_WIDTH  128
#define DISPLAY_HEIGHT 128


ESP32Time rtcESP(-3600);  // offset 1 hour from NTP time

SdFat                SD;         // SD card filesystem
Adafruit_ImageReader reader(SD); // Image-reader object, pass in SD filesys

Adafruit_ST7735      tft    = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
AnimatedGIF gif;
File gifFile;

String folder = GIF_FOLDER;
String transitionGifName = "";
String previousTransitionGifName = "";

void rcPrintLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "rcPrintLocalTime(): %A, %B %d %Y %H:%M:%S");
}

bool rcSetTransitionGIFName()
{

char hm[6];
bool validTime = false;
int thisYear = 9999;

#ifdef ENABLE_RTC_PCF8523
  if (validRtc8523) {
    DateTime now = rtc8523.now();

    if (now.year() > 2022) {
        sprintf(hm, "%02d%02d", now.hour(), now.minute());
        validTime = true;
    } else {
      //Serial.println("rtc8523.now() returned invalid time");
    }
  }
#endif

  thisYear = year();

  if (!validTime && thisYear > 2022) {
    //Serial.printf("valid year=%d\n", thisYear);
    sprintf(hm, "%02d%02d", hour(), minute());
    validTime = true;
  } else {
    //Serial.println("year() returned invalid time");
  }

  if (!validTime) {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo))
    {
      thisYear = timeinfo.tm_year + 1900;
      if (thisYear > 2022)
      {
        //Serial.printf("valid timeinfo.tm_year=%d set time to %02d:%02d\n", thisYear, timeinfo.tm_hour, timeinfo.tm_min);

        sprintf(hm, "%02d%02d", timeinfo.tm_hour, timeinfo.tm_min);

        validTime = true;
      } else{
        Serial.printf("timeinfo.tm_year is not valid %d\n", thisYear);
      }

    } else {
      Serial.println("ERROR. getLocalTime() Failed.");
    }
    //sprintf(hm, "%02d%02d", timeinfo.tm_hour, timeinfo.tm_min);
  }

  if (!validTime)
  {
    Serial.println("ERROR. Failed to obtain time. Not setting filename.");
    return false;
  }

  //Serial.printf("hm = %s.  Setting file to rubiks-clock-%s.gif\n", hm, hm);

  String hourmin = String(hm);


  transitionGifName = folder + "rubiks-clock-" + hourmin + ".gif";

  return true;
}

#ifdef ENABLE_WIFI

void readWiFiConfig()
{

  // File format:
  // ssid=yourssidname
  // password=your password
  // lines that begin with # are ignored
  // no space after "=" unless it is part of the password value

  //Serial.print("Open file: ");
  //Serial.println(fname);

  int found = 0;
  int foundSSID = 0;
  int foundPW = 0;
  String wifiConf;
  String ssid = "";
  String pwd = "";
  char line[128];
  int n;

  Serial.printf("reading wifi.conf..\n");
  // open test file
  SdFile file("/wifi.conf", O_READ);
  
  // check for open error
  if (!file.isOpen()) {
    Serial.println("ERROR: cannot open file wifi.conf root of sd card");
    return;
  }
 
  // read lines from the file
  while ((n = file.fgets(line, sizeof(line))) > 0) {
    if (line[0] == '#') continue;
    if (line[n - 1] == '\n') {
      String lineString = String(line);

      int indexOfEqual = lineString.indexOf('=');
      int indexOfCR = lineString.indexOf('\n');

      if (lineString.startsWith("ssid")) {
        ssid = lineString.substring(indexOfEqual+1, indexOfCR);
        foundSSID = 1;
      } else if (lineString.startsWith("password")) {
        pwd = lineString.substring(indexOfEqual+1, indexOfCR);
        foundPW = 1;
      }
    } else {
      // no '\n' - line too long or missing '\n' at EOF
      Serial.printf("ERROR ignoring bad line %s\n", line);
      continue;
    }
    if (foundSSID && foundPW) {
      found = 1;
      break;
    }
  }

  file.close();

  Serial.printf("ssid = .%s.\n", ssid.c_str());
  //Serial.printf("password = .%s.\n", pwd.c_str());

  if (found) {
    wifiSsid = ssid;
    wifiPassword = pwd;    
  } else {
    Serial.printf("ERROR SSID or password not found in wifi.conf. Using default\n");
  }

} /* readWiFiConfig() */
#endif // ENABLE_WIFI

bool isInt(const String s){
  return !isnan(s.toInt());
}

// search CSV file for the string.  set TZ to value after comma
//  Format of CSV every line looks something like this:
//  "America/New_York","EST5EDT,M3.2.0,M11.1.0"

bool rcFindTZInFile(const String tzHumanString) {

  bool found = false;
  String tz = "";
  char line[128];
  int n;
  Serial.printf("rcFindTZInFile()\n");

  String searchStr = String("\"") + tzHumanString + "\"";

  Serial.printf("reading zones.csv..\n");
  // open test file
  SdFile file("/zones.csv", O_READ);
  
  // check for open error
  if (!file.isOpen()) {
    Serial.println("ERROR: cannot open file zones.csv root of sd card");
    return false;
  }

  // read lines from the file
  while ((n = file.fgets(line, sizeof(line))) > 0) {
    if (line[0] == '#') continue;
    if (line[n - 1] == '\n') {
      String lineString = String(line);

      int indexOfComma = lineString.indexOf(',');
      int indexOfCR = lineString.indexOf('\n');

      if (indexOfComma < 0 || indexOfCR < 0) {
        // no comma in this line
        continue;
      }

      if (lineString.startsWith(searchStr)) {
        tz = lineString.substring(indexOfComma+1, indexOfCR);
        found = true;
        break;
      }
    } else {
      // no '\n' - line too long or missing '\n' at EOF
      Serial.printf("ERROR ignoring bad line %s\n", line);
      continue;
    }
  }

  file.close();

  //Serial.printf("tz = %s\n", tz.c_str());

  if (found) {
    if (tz[0] == '\"') {
      // remove quotes
      timezoneStr = tz.substring(1,tz.length()-1);
    } else {
      timezoneStr = tz;
    }

  } else {
    Serial.printf("ERROR timezone not found in file.  Using default.\n");
  }

  Serial.printf("timezone = %s\n", timezoneStr.c_str());

  return found;
}

void rcReadTimeConfig()
{

  // read file timezone.conf.
  //  Contents are:
  //    TZ: timezone from https://github.com/nayarsystems/posix_tz_db/blob/master/zones.json
  //    DS: daylight savings offset (e.g 0)
  // File format:
  // TZ=America/New_York
  // DS=0
  // lines that begin with # are ignored
  // no space after "=" unless it is part of the password value

  //Serial.print("Open file: ");
  //Serial.println(fname);

  int foundIntTZ = false;
  int foundTZ = false;
  int foundDS = false;
  //String wifiConf;
  String tz = "";
  String ds = "";
  String ntp = "";
  char line[128];
  int n;

  Serial.printf("reading time.conf..\n");
  // open test file
  SdFile file("/time.conf", O_READ);
  
  // check for open error
  if (!file.isOpen()) {
    Serial.println("ERROR: cannot open file time.conf root of sd card");
    return;
  }
 
  // read lines from the file
  while ((n = file.fgets(line, sizeof(line))) > 0) {
    if (line[0] == '#') continue;
    if (line[n - 1] == '\n') {
      String lineString = String(line);

      int indexOfEqual = lineString.indexOf('=');
      int indexOfCR = lineString.indexOf('\n');

      if (lineString.startsWith("TZ")) {
        tz = lineString.substring(indexOfEqual+1, indexOfCR);
        Serial.printf("TZ = %s\n", tz.c_str());

        if (isInt(tz)){
          gmtOffset_sec =tz.toInt()*60*60;
          foundIntTZ = true;
        } else {
          //search for string in zones.csv
          //if (!rcFindTZInFile(tz)) {
          //  tz = timezoneStr;
          //}
          foundTZ = true;
        }

      } else if (lineString.startsWith("DS")) {
        ds = lineString.substring(indexOfEqual+1, indexOfCR);
        Serial.printf("DS = %s\n", ds.c_str());
        if (isInt(ds)){
          daylightOffset_sec = ds.toInt()*60*60;    
          foundDS = true;
        }

      } else if (lineString.startsWith("NTP")) {
        ntp = lineString.substring(indexOfEqual+1, indexOfCR);
        Serial.printf("ntp server is = .%s.\n", ntp.c_str());
        strcpy(ntpServer, ntp.c_str());
      }

    } else {
      // no '\n' - line too long or missing '\n' at EOF
      Serial.printf("ERROR ignoring bad line %s\n", line);
      continue;
    }
    if ((foundIntTZ || foundTZ) && foundDS) {
      break;
    }
  }

  file.close();

  if (!rcFindTZInFile(tz)) {
    timezoneStr = tz;
  }
} /* rcReadTimeConfig() */

void setTimezone(String timezone){
  Serial.printf("  Setting Timezone to %s\n",timezone.c_str());
  setenv("TZ",timezone.c_str(),1);  //  Now adjust the TZ.  Clock settings are adjusted to show the new local time
  tzset();
}

#ifdef ENABLE_SLEEP
/*
Method to print the touchpad by which ESP32
has been awaken from sleep
*/
void print_wakeup_touchpad(){
  touchPin = esp_sleep_get_touchpad_wakeup_status();

  #if CONFIG_IDF_TARGET_ESP32
    switch(touchPin)
    {
      case 0  : Serial.println("Touch detected on GPIO 4"); break;
      case 1  : Serial.println("Touch detected on GPIO 0"); break;
      case 2  : Serial.println("Touch detected on GPIO 2"); break;
      case 3  : Serial.println("Touch detected on GPIO 15"); break;
      case 4  : Serial.println("Touch detected on GPIO 13"); break;
      case 5  : Serial.println("Touch detected on GPIO 12"); break;
      case 6  : Serial.println("Touch detected on GPIO 14"); break;
      case 7  : Serial.println("Touch detected on GPIO 27"); break;
      case 8  : Serial.println("Touch detected on GPIO 33"); break;
      case 9  : Serial.println("Touch detected on GPIO 32"); break;
      default : Serial.println("Wakeup not by touchpad"); break;
    }
  #else
    if(touchPin < TOUCH_PAD_MAX)
    {
      Serial.printf("Touch detected on GPIO %d\n", touchPin); 
    }
    else
    {
      Serial.println("Wakeup not by touchpad");
    }
  #endif
}

/*
Method to print the reason by which ESP32
has been awaken from sleep
*/
bool getWakeupReason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  bool didWakeFromSleep = true;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : didWakeFromSleep = false; Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }

  return (didWakeFromSleep);

}
#endif // ENABLE_SLEEP

// Much of the following GIF processing comes from Adafruit examples.

void * GIFOpenFile(const char *fname, int32_t *pSize)
{

  Serial.printf("opening gif %s..\n", fname);

  gifFile = SD.open(fname, FILE_READ);
  if (gifFile)
  {
    *pSize = gifFile.size();
    return (void *)&gifFile;
  }
  return NULL;
} /* GIFOpenFile() */

void GIFCloseFile(void *pHandle)
{
  File *gifFile = static_cast<File *>(pHandle);
  if (gifFile != NULL)
     gifFile->close();
} /* GIFCloseFile() */

int32_t GIFReadFile(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen)
{
    int32_t iBytesRead;
    iBytesRead = iLen;
    File *f = static_cast<File *>(pFile->fHandle);
    // Note: If you read a file all the way to the last byte, seek() stops working
    if ((pFile->iSize - pFile->iPos) < iLen)
       iBytesRead = pFile->iSize - pFile->iPos - 1; // <-- ugly work-around
    if (iBytesRead <= 0)
       return 0;
    iBytesRead = (int32_t)f->read(pBuf, iBytesRead);
    pFile->iPos = f->position();
    return iBytesRead;
} /* GIFReadFile() */

int32_t GIFSeekFile(GIFFILE *pFile, int32_t iPosition)
{ 
  int i = micros();
  File *f = static_cast<File *>(pFile->fHandle);
  f->seek(iPosition);
  pFile->iPos = (int32_t)f->position();
  i = micros() - i;
//  Serial.printf("Seek time = %d us\n", i);
  return pFile->iPos;
} /* GIFSeekFile() */

// Draw a line of image directly on the LCD
void GIFDraw(GIFDRAW *pDraw)
{
    uint8_t *s;
    uint16_t *d, *usPalette, usTemp[320];
    int x, y, iWidth;

    iWidth = pDraw->iWidth;
    if (iWidth + pDraw->iX > DISPLAY_WIDTH)
       iWidth = DISPLAY_WIDTH - pDraw->iX;
    usPalette = pDraw->pPalette;
    y = pDraw->iY + pDraw->y; // current line
    if (y >= DISPLAY_HEIGHT || pDraw->iX >= DISPLAY_WIDTH || iWidth < 1)
       return; 
    s = pDraw->pPixels;
    if (pDraw->ucDisposalMethod == 2) // restore to background color
    {
      for (x=0; x<iWidth; x++)
      {
        if (s[x] == pDraw->ucTransparent)
           s[x] = pDraw->ucBackground;
      }
      pDraw->ucHasTransparency = 0;
    }

    // Apply the new pixels to the main image
    if (pDraw->ucHasTransparency) // if transparency used
    {
      uint8_t *pEnd, c, ucTransparent = pDraw->ucTransparent;
      int x, iCount;
      pEnd = s + iWidth;
      x = 0;
      iCount = 0; // count non-transparent pixels
      while(x < iWidth)
      {
        c = ucTransparent-1;
        d = usTemp;
        while (c != ucTransparent && s < pEnd)
        {
          c = *s++;
          if (c == ucTransparent) // done, stop
          {
            s--; // back up to treat it like transparent
          }
          else // opaque
          {
             *d++ = usPalette[c];
             iCount++;
          }
        } // while looking for opaque pixels
        if (iCount) // any opaque pixels?
        {
          tft.startWrite();
          tft.setAddrWindow(pDraw->iX+x, y, iCount, 1);
          tft.writePixels(usTemp, iCount, false, false);
          tft.endWrite();
          x += iCount;
          iCount = 0;
        }
        // no, look for a run of transparent pixels
        c = ucTransparent;
        while (c == ucTransparent && s < pEnd)
        {
          c = *s++;
          if (c == ucTransparent)
             iCount++;
          else
             s--; 
        }
        if (iCount)
        {
          x += iCount; // skip these
          iCount = 0;
        }
      }
    }
    else
    {
      s = pDraw->pPixels;
      // Translate the 8-bit pixels through the RGB565 palette (already byte reversed)
      for (x=0; x<iWidth; x++)
        usTemp[x] = usPalette[*s++];
      tft.startWrite();
      tft.setAddrWindow(pDraw->iX, y, iWidth, 1);
      tft.writePixels(usTemp, iWidth, false, false);
      tft.endWrite();
    }
} /* GIFDraw() */

void playGif(String filename) {
    if (gif.open(filename.c_str(), GIFOpenFile, GIFCloseFile, GIFReadFile, GIFSeekFile, GIFDraw))
    {
      GIFINFO gi;
      Serial.printf("Successfully opened GIF; Canvas size = %d x %d\n", gif.getCanvasWidth(), gif.getCanvasHeight());
      if (gif.getInfo(&gi)) {
        Serial.printf("frame count: %d\n", gi.iFrameCount);
        Serial.printf("duration: %d ms\n", gi.iDuration);
        Serial.printf("max delay: %d ms\n", gi.iMaxDelay);
        Serial.printf("min delay: %d ms\n", gi.iMinDelay);
      }
      while (gif.playFrame(true, NULL))
      {
        // TODO maybe insert callback so some quick function can run between frames
      }
      gif.close();
      
    }
}
#ifdef INITIALIZE_RTC
#ifdef ENABLE_RTC_ESP32
const char *monthName[12] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

typedef struct {
  int year, month, day, hour, minute, second;
} MyTime;

MyTime myTime;

bool rcGetTime(const char *str)
{
  int Hour, Min, Sec;

  if (sscanf(str, "%d:%d:%d", &Hour, &Min, &Sec) != 3) return false;
  myTime.hour = Hour;
  myTime.minute = Min;
  myTime.second = Sec;
  return true;
}

bool rcGetDate(const char *str)
{
  char Month[12];
  int Day, Year;
  uint8_t monthIndex;

  if (sscanf(str, "%s %d %d", Month, &Day, &Year) != 3) return false;
  for (monthIndex = 0; monthIndex < 12; monthIndex++) {
    if (strcmp(Month, monthName[monthIndex]) == 0) break;
  }
  if (monthIndex >= 12) return false;
  myTime.day = Day;
  myTime.month = monthIndex + 1;
  myTime.year = Year;
  return true;
}
#endif // ENABLE_RTC_ESP32
#endif // INITIALIZE_RTC

////////////////////////////////////////////////////////
//
// PrintTime
//
// Serial Print time (DD/MM/YYYY - HH:MM:SS)
//
void rcPrintTime() {
  char cTime[64];
  
    sprintf(cTime, "day() %02u/%02u/%4u - %02u:%02u:%02u", month(), day(), year(), hour(), minute(), second());
    Serial.println(cTime);

  if (validRtc8523) {
    DateTime now = rtc8523.now();
    sprintf(cTime, "rtc8523: %02u/%02u/%4u - %02u:%02u:%02u", now.month()+1, now.day(), now.year(), now.hour(), now.minute(), now.second());
    Serial.println(cTime);
  }
}

#ifdef ENABLE_WIFI
bool rcWifiConnected()
{
  return (WiFi.status() == WL_CONNECTED);
}

bool rcWiFiWasConnected = false;

// Connect to wifi and get time from NTP server
void rcConnectToWiFi()
{
  //connect to WiFi
  Serial.printf("\nConnecting to %s\n", wifiSsid.c_str());
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiSsid.c_str(), wifiPassword.c_str());

  int connected = 0;
  int i;
  // 20 second timeout
  for (i=0; i<20; i++) {
    if (rcWifiConnected()) {
      connected = 1;
      break;
    }
    delay(1000);
    Serial.print(".");
  }

  if (connected) {
    Serial.println(" CONNECTED");

    rcWiFiWasConnected = true;

    //init and get the time
    configTime(0, 0, ntpServer, ntpServer1, ntpServer2);
    setTimezone(timezoneStr);

    rcPrintLocalTime();

    struct tm timeinfo;
    if (getLocalTime(&timeinfo)){
      Serial.printf("Initializing RTC from NTP server\n");
#ifdef ENABLE_RTC_ESP32
      rtcESP.setTimeStruct(timeinfo);
#else
      DateTime now =  DateTime(timeinfo.tm_year+1900,timeinfo.tm_mon,timeinfo.tm_mday,
                              timeinfo.tm_hour,timeinfo.tm_min,timeinfo.tm_sec);
      if (validRtc8523){
        char cTime[64];

        Serial.printf("adjust rtc8523 time\n");
        sprintf(cTime, "set rtc8523 time: %02u/%02u/%4u - %02u:%02u:%02u", now.day(), now.year(), now.hour(), now.minute(), now.second());

        rtc8523.adjust(DateTime(now));
      }else {
        Serial.printf("adjust rtcESP time\n");
        rtcESP.setTimeStruct(timeinfo);
      }
#endif
    }
    //disconnect WiFi as it's no longer needed
    //WiFi.disconnect(true);
    //WiFi.mode(WIFI_OFF);
  } else {
    Serial.printf("\nERROR: WiFi timeout.  No time set.\n");
  }
}
#endif // ENABLE_WIFI

////////////////////////////////////////////////////
// Arduino initialization in setup()
////////////////////////////////////////////////////

void setup() {

  bool didWakeFromSleep = false;

  Serial.begin(115200);
  delay(1000); //Take some time to open up the Serial Monitor

  Serial.println("setup()");
  //Wire.begin(I2C_SDA, I2C_SCL);

  pinMode(TFT_BACKLIGHT, OUTPUT);    // low = off, high = on

#ifdef ENABLE_SLEEP
  //Increment boot number and print it every reboot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  //gpio_hold_dis(GPIO_NUM_37);
  gpio_deep_sleep_hold_en();

//Setup interrupt on Touch Pad 5 (GPIO12)
//touchAttachInterrupt(TOUCH_PIN, callback, THRESHOLD); 

  //Print the wakeup reason for ESP32
  didWakeFromSleep = getWakeupReason();

  if (didWakeFromSleep) {
    print_wakeup_touchpad();
  }

  touchSleepWakeUpEnable(TOUCH_PIN,THRESHOLD);


  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +
  " Seconds");

#endif // ENABLE_SLEEP

  if (didWakeFromSleep) {
    Serial.println("turning on display");
    digitalWrite(TFT_BACKLIGHT, HIGH);
    Serial.println("We woke from sleep.  skip the rest of setup and go directly to loop()");

    return;

  } else {
    Serial.println("Normal path.  We did not wake from sleep.");
  }


  //pinMode(SD_CS, OUTPUT);    // sets the digital pin 13 as output
  //pinMode(TFT_CS, OUTPUT);    // sets the digital pin 13 as output

  // initialize chip select pins to high to avoid SPI confusion.  AKA disable both for now.
  //digitalWrite(SD_CS, LOW);
  //digitalWrite(TFT_CS, LOW);
  //rcPrintLocalTime();

  //Serial.println("while(1)");
  //while(1);

  ImageReturnCode stat; // Status from image-reading functions

// Note - some systems (ESP32?) require an SPI.begin() before calling SD.begin()

  if(!SD.begin(SD_CS)) { // Breakouts require 10 MHz limit due to longer wires
    Serial.println("SD Card mount failed!");
    return;
  }
  else
  {
    Serial.println("SD Card mount succeeded!");
  }

  rcReadTimeConfig();

  tft.initR(INITR_144GREENTAB); // Initialize screen
  tft.fillScreen(ST7735_BLACK);

  //stat = reader.drawBMP("/splashscreen.bmp", tft, 0, 0);

  gif.begin(LITTLE_ENDIAN_PIXELS);

  reader.printStatus(stat);   // How'd we do?

  // Show an intro gif of a cube floating into place

  Serial.println("turning on display");
    digitalWrite(TFT_BACKLIGHT, HIGH);
  
  if (!didWakeFromSleep) {
    playGif("/cubedemo.gif");
  }
 

#ifdef INITIALIZE_RTC

#ifdef ENABLE_RTC_ESP32
  // Initialize with build date for now
  rcGetDate(__DATE__);
  rcGetTime(__TIME__);
#endif // ENABLE_RTC_ESP32

#ifdef ENABLE_RTC_ESP32
  rtcESP.setTime(myTime.second, myTime.minute, myTime.hour, myTime.day, myTime.month, myTime.year);  // build time
#endif

#ifdef ENABLE_RTC_PCF8523
  Wire.begin(I2C_SDA, I2C_SCL);
  //return;
  delay (1000);
  for (int i=0; i<3; i++) {
    if (rtc8523.begin()) {
      validRtc8523 = true;
      Serial.println("Found RTC 8523");
      break;
    } else {
      validRtc8523 = false;
      Serial.println("Couldn't find RTC 8523");
      Serial.flush();
      sleep (1);
      //while (1) delay(10);
    }
  }
  if (validRtc8523){

    bool shouldInitRTC = ! rtc8523.initialized() || rtc8523.lostPower();
    //shouldInitRTC = true;

    if (shouldInitRTC) {
      Serial.println("RTC is NOT initialized, let's set the time!");
      // When time needs to be set on a new device, or after a power loss, the
      // following line sets the RTC to the date & time this sketch was compiled

      DateTime buildTime = DateTime(F(__DATE__), F(__TIME__));
  // maybe takes 2 minute to load and run after compile saw this file
  
      //DateTime maybeNow = buildTime + TimeSpan(0, 1, 5, 0);
      DateTime maybeNow = buildTime + TimeSpan(120);

      rtc8523.adjust(DateTime(maybeNow));
      // This line sets the RTC with an explicit date & time, for example to set
      // January 21, 2014 at 3am you would call:
      // rtc8523.adjust(DateTime(2014, 1, 21, 3, 0, 0));
      //
      // Note: allow 2 seconds after inserting battery or applying external power
      // without battery before calling adjust(). This gives the PCF8523's
      // crystal oscillator time to stabilize. If you call adjust() very quickly
      // after the RTC is powered, lostPower() may still return true.
    }

    // When time needs to be re-set on a previously configured device, the
    // following line sets the RTC to the date & time this sketch was compiled
    // rtc8523.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc8523.adjust(DateTime(2014, 1, 21, 3, 0, 0));

    // When the RTC was stopped and stays connected to the battery, it has
    // to be restarted by clearing the STOP bit. Let's do this to ensure
    // the RTC is running.
    rtc8523.start();
  }
#endif // ENABLE_RTC_PCF8523

#endif //INITIALIZE_RTC

#ifdef ENABLE_WIFI


  //if (!didWakeFromSleep) {

  //Serial.println("Read WiFi configuration..");
  //sleep(1);

  readWiFiConfig();

  //rcConnectToWiFi();


#endif // ENABLE_WIFI

}

////////////////////////////////////////////////////
// Arduino endless loop in loop()
////////////////////////////////////////////////////

void loop() {

  static bool triedWiFiJustNow = false;
  bool validYear = false;

  static unsigned long previousMinute=0;
  static unsigned long previousHour=0;
  static unsigned long previousDay=0;

  DateTime now;
  if (validRtc8523) {
    now = rtc8523.now();

    if (now.year() < 2022) {
      //Serial.println("rtc8523.now() returned invalid time");
      validYear = false;
    } else {
      validYear = true;
    }

    if (validYear && previousMinute != now.minute()) {
      previousMinute = now.minute();
      //rcPrintTime();
    }
  }
 

  if (!triedWiFiJustNow && !rcWiFiWasConnected) {
      Serial.println("WiFi was not connected.  Attempt to connect now.");
      rcConnectToWiFi();
      triedWiFiJustNow = true;
  }

  bool timeChanged =
    rcSetTransitionGIFName();

  if (timeChanged && !transitionGifName.equals(previousTransitionGifName)) {

      countSinceSleep++;

      Serial.println ("Time changed!");
      previousTransitionGifName = transitionGifName;

      rcPrintLocalTime();

      playGif(transitionGifName);

  if (validRtc8523 && validYear && previousHour != now.hour()) {
    previousHour = now.hour();
    Serial.println("Hour changed. update from NTP");
    if (!rcWiFiWasConnected) {
      Serial.println("WiFi was not connected.  Attempt to connect now.");
      rcConnectToWiFi();
      triedWiFiJustNow = true;
    }

    if (validYear && previousDay != now.day()) {
      previousDay = now.day();
      if (!triedWiFiJustNow) {
        Serial.println("Day changed. update from NTP");
        rcConnectToWiFi();
        triedWiFiJustNow = true;
      }
    }
  }


#ifdef ENABLE_SLEEP
      if (countSinceSleep > WAKE_TIME) {
          Serial.println("turning off display");
          digitalWrite(TFT_BACKLIGHT, LOW);
          //tft.initR(INITR_144GREENTAB); // Initialize screen
          //tft.fillScreen(ST7735_BLACK);
          //sleep (5);
          Serial.println("Going to sleep now");
          delay(1000);
          Serial.flush(); 
          esp_deep_sleep_start();
      }
#endif // ENABLE_SLEEP

  }
}
