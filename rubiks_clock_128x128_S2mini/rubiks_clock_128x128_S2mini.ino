
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

// The real-time-clock can be an external PCF8532 or the internal ESP32 RTC.
// Define only one of these.

#define ENABLE_RTC_PCF8523
//#define ENABLE_RTC_ESP32

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

#ifdef ENABLE_RTC_ESP32
#include <ESP32Time.h>
#include <TimeLib.h>
#endif
#ifdef ENABLE_RTC_PCF8523
#include "RTClib.h"
#endif


// define this to set the real time clock to be build date
#define INITIALIZE_RTC

#define ENABLE_WIFI

#ifdef ENABLE_WIFI
// It is recommended to modify wifi.conf with your network credentials.
// but you can choose to set a default network name and password if you
// prefer.  If so, then remove wifi.conf from the sdcard.

String wifiSsid     = "ssid";
String wifiPassword = "password";
#endif // ENABLE_WIFI

#ifdef ENABLE_RTC_PCF8523
#define I2C_SDA 33
#define I2C_SCL 35

RTC_PCF8523 rtc;
#endif //ENABLE_RTC_PCF8523

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

#define SD_CS     16 // SD card select pin
#define TFT_CS    12 // TFT select pin
#define TFT_RST    5 // TFT reset pin
#define TFT_DC     3 // TFT Data/Command pin


#define DISPLAY_WIDTH  128
#define DISPLAY_HEIGHT 128


#ifdef ENABLE_RTC_ESP32
ESP32Time rtc(-120);  // offset 2 minutes difference build time to boot time
#endif // ENABLE_RTC_ESP32

SdFat                SD;         // SD card filesystem
Adafruit_ImageReader reader(SD); // Image-reader object, pass in SD filesys

Adafruit_ST7735      tft    = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
AnimatedGIF gif;
File gifFile;

String folder = GIF_FOLDER;
String filename = "";
String previousFilename = "";

void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
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

bool getTime(const char *str)
{
  int Hour, Min, Sec;

  if (sscanf(str, "%d:%d:%d", &Hour, &Min, &Sec) != 3) return false;
  myTime.hour = Hour;
  myTime.minute = Min;
  myTime.second = Sec;
  return true;
}

bool getDate(const char *str)
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

////////////////////////////////////////////////////
// Arduino initialization in setup()
////////////////////////////////////////////////////

void setup() {

  ImageReturnCode stat; // Status from image-reading functions

#ifdef INITIALIZE_RTC

#ifdef ENABLE_RTC_ESP32
  // Initialize with build date for now
  getDate(__DATE__);
  getTime(__TIME__);
#endif // ENABLE_RTC_ESP32

#ifdef ENABLE_RTC_ESP32
  rtc.setTime(myTime.second, myTime.minute, myTime.hour, myTime.day, myTime.month, myTime.year);  // build time
#endif

#ifdef ENABLE_RTC_PCF8523
  Wire.begin(I2C_SDA, I2C_SCL);
  //return;
  delay (1000);
    if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }


  if (! rtc.initialized() || rtc.lostPower()) {
    Serial.println("RTC is NOT initialized, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    //
    // Note: allow 2 seconds after inserting battery or applying external power
    // without battery before calling adjust(). This gives the PCF8523's
    // crystal oscillator time to stabilize. If you call adjust() very quickly
    // after the RTC is powered, lostPower() may still return true.
  }

  // When time needs to be re-set on a previously configured device, the
  // following line sets the RTC to the date & time this sketch was compiled
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // This line sets the RTC with an explicit date & time, for example to set
  // January 21, 2014 at 3am you would call:
  // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));

  // When the RTC was stopped and stays connected to the battery, it has
  // to be restarted by clearing the STOP bit. Let's do this to ensure
  // the RTC is running.
  rtc.start();
#endif // ENABLE_RTC_PCF8523

/*---------set with NTP---------------*/
//  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
//  struct tm timeinfo;
//  if (getLocalTime(&timeinfo)){
//    rtc.setTimeStruct(timeinfo); 
//  }

#endif //INITIALIZE_RTC

// Note - some systems (ESP32?) require an SPI.begin() before calling SD.begin()
// this code was tested on a Teensy 4.1 board
  //if(!SD.begin(SD_CS, SD_SCK_MHZ(10))) { // Breakouts require 10 MHz limit due to longer wires
  if(!SD.begin(SD_CS)) { // Breakouts require 10 MHz limit due to longer wires
    Serial.println("SD Card mount failed!");
    return;
  }
  else
  {
    Serial.println("SD Card mount succeeded!");
  }

  tft.initR(INITR_144GREENTAB); // Initialize screen
  tft.fillScreen(ST7735_BLACK);

  //stat = reader.drawBMP("/splashscreen.bmp", tft, 0, 0);

  gif.begin(LITTLE_ENDIAN_PIXELS);

  reader.printStatus(stat);   // How'd we do?

  // Show an intro gif of a cube floating into place
    //if (gif.open("/cubetransition.gif", GIFOpenFile, GIFCloseFile, GIFReadFile, GIFSeekFile, GIFDraw))
    if (gif.open("/cubedemo.gif", GIFOpenFile, GIFCloseFile, GIFReadFile, GIFSeekFile, GIFDraw))
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
      }
      gif.close();
      
    }

#ifdef ENABLE_WIFI

  readWiFiConfig();

  //connect to WiFi
  Serial.printf("\nConnecting to %s\n", wifiSsid.c_str());
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiSsid, wifiPassword);

  int connected = 0;
  int i;
  // 20 second timeout
  for (i=0; i<20; i++) {
    if (WiFi.status() == WL_CONNECTED) {
      connected = 1;
      break;
    }
    delay(1000);
    Serial.print(".");
  }

  if (connected) {
    Serial.println(" CONNECTED");

    //init and get the time
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    printLocalTime();

    //disconnect WiFi as it's no longer needed
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
  } else {
    Serial.printf("\nERROR: WiFi timeout. Unable to reach NTP server. Time may not be correct.\n");
  }


#endif // ENABLE_WIFI

}

////////////////////////////////////////////////////
// Arduino endless loop in loop()
////////////////////////////////////////////////////

void loop() {


#ifdef ENABLE_RTC_ESP32
  //filename = rtc.getTime("/12hourclock-200x120/rubiks-clock-%H%M.gif");
  filename = rtc.getTime("/12hourclock-128x128/rubiks-clock-%H%M.gif");
#endif

#ifdef ENABLE_RTC_PCF8523
    DateTime now = rtc.now();

    int hour = now.hour();
    int minute = now.minute();
    char hm[6];
    sprintf(hm, "%02d%02d", hour, minute);
    String hourmin = String(hm);
    filename = folder + "rubiks-clock-" + hourmin + ".gif";

#endif

  if (!filename.equals(previousFilename)) {
      Serial.println ("Time changed!");
      previousFilename = filename;

      printLocalTime();

  //  if (gif.open("/12hourclock-200x120/rubiks-clock-0002.gif", GIFOpenFile, GIFCloseFile, GIFReadFile, GIFSeekFile, GIFDraw))
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
      }
      gif.close();
      
    }
    else
    {
      Serial.printf("Error opening file = %d\n", gif.getLastError());
      while (1)
      {

      };
    }
  }

}
