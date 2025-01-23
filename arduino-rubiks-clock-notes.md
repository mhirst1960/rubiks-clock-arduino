# Arduino Rubik's Clock

![Alt Text](sdcard_files/12hourclock-128x128/rubiks-clock-1002.gif)

# parts

- Wemos S2 mini
- USB C cable to your PC
- Adafruit 1.44" Color TFT with Micro SD Socket
- 4GB or larger Micro SD Card
- Adafruit PCF8532 RTC QT, Real Time Clock
- 
- breadboard
- jumper wires

# Links

## Wemos ESP32 computer

Amazon: https://www.amazon.com/dp/B0B291LZ99

https://www.wemos.cc/en/latest/s2/s2_mini.html

https://www.wemos.cc/en/latest/tutorials/s2/get_started_with_arduino_s2.html

## TFT display

Amazon: https://www.amazon.com/dp/B00SK6932C

https://learn.adafruit.com/adafruit-1-44-color-tft-with-micro-sd-socket


## Ardiuno setup

### install esp32 by Espressif Systems
1. Add this link to preferences -> settings -> additional boards manager links:
  https://espressif.github.io/arduino-esp32/package_esp32_index.json

  https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html

2. Tools -> board -> EPS32 -> LOLIN S2 MINI

### test blink app

### blink sketch
File -> examples -> basic -> blink

### enable programming on Purple S2 Mini

Connect to PC with USB-C

1. press and hold switch 0
2. press and release RST
3. release switch 0

Press the "Upload" arrow on the Arduino IDE

after uploading the blue LED will steadily blink forever

## Wiring TFT

### LOLIN S2 mini board
Solder only outside two connectors to fit on an experimenter development board.


### lolin_s2_mini
    static const uint8_t SS    = 12; # CS
    static const uint8_t MOSI  = 11;
    static const uint8_t MISO  = 9;
    static const uint8_t SCK   = 7;

### Adafruit 128x128 TFT test

File -> examples→Adafruit ImageReader Library -> BreakoutST7735 - 128x128

Modify BreakoutST7735-128x128.ino setting sdcard/TFT SPI pins to this

    #define SD_CS    16 // SD card select pin
    #define TFT_CS  12  // TFT select pin
    #define TFT_DC   3  // TFT display/command pin
    #define TFT_RST  5  // Or set to -1 and connect to Arduino RESET pin


| description                 | S2 Mini | Adafruit ST7735 TFT |
| ----                         | ------- | -------------- |
| 3v or 5V power               | 3v3 | Vin 1 |
| 3.3V out                     | - | 3v3 2 |
| Ground                       | GND | Gnd 3 |
| SPI clock                    | 7 | SCK 4 |
| MISO (for SD card only)      | 9 | SO 5 |
| MOSI                         | 11 | SI 6 |
| TFT_CS chip select           | 12 (SS) | TCS 7 |
| TFT reset                    | 5 | RST 8 |
| TFT SPI data/command select  | 3 | D/C 9 |
| SD card chip select          | 16 | CCS 10 |
| backlight PWM                | - | Lite 11 |


## play GIF

### install esp-chimera-core

in Arduino library manager search for "esp chimera". Install ESP32-Chimera-Core by tobozo,Lovyan03

install all

### install AnimatedGIF
from Library install AnimatedGIF by Larry Bank

File -> Examples -> AnimatedGIF -> play_all_sd_files

modify this line:

  char *szDir = "/GIF"; // play all GIFs in this directory on the SD card

to specify the directory of the 12-hour clock gIFS:

  char *szDir = "/12hourclock-200x120"; // play all GIFs in this directory on the SD card


Maybe this will work:
- https://github.com/bitbank2/AnimatedGIF/blob/master/examples/play_all_sd_files/play_all_sd_files.ino
- https://github.com/bitbank2/AnimatedGIF/blob/master/examples/play_all_sd_files/play_all_sd_files.ino

Or this:
- https://github.com/adafruit/Adafruit_Arcada_GifDecoder

## Cube gif testing

Make sure to test the midnight cube.  All the transitions take 5 seconds or less but 12:00 or 00:00 midnight takes 15 or 20 seconds to solve itself.

After that, on the host computer, sort all the images by size and test the largest images as these will be the ones that have the most twists.  It should be 7:50 pm, and 19:19.

### midnight 12:00 am:
/12hourclock-128x128/rubiks-clock-0000.gif

### midnight 00:00:
/24hourclock-128x128/rubiks-clock-0000.gif

### 7:50 pm:
/12hourclock-128x128/rubiks-clock-1950.gif

### 1919:
/24hourclock-128x128/rubiks-clock-1919.gif


## Generating the splashscreen image of a static Rubik's clock

1. crop image of rubik's Cube to 128x128 pixels and save as png
2. Use ImageMagic convert to convert to 8-bit bmp file like this:
   
  convert splashscreen.png -type truecolor splashscreen.bmp

## Create Splashscreen Demo

1. grab image of a cube.
2. import to iMovie and crop and stitch as you like (Ken burns cropping with extreme zoom might be an interesting effect)
3. export mp4
4. ffmpeg  -i  cubedemo.mp4 -vf "fps=25,crop=iw-320:ih:120:0,scale=128:128:flags=lanczos,palettegen"  -y palette.png
5. ffmpeg -i cubedemo.mp4 -i  palette.png -lavfi fps=25,crop=iw-320:ih:120:0,scale=128:128:flags=lanczos -y cubedemo.gif
6. copy cubedemo.gif to sdcard

## Case

Here's a possibility

https://www.thingiverse.com/thing:500966/files
