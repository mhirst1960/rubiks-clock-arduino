# Arduino Rubik's Clock

The Arduino Rubik's Clock is a clock that shows the time of day on the face of a Rubik's Cube.  Every minute of the day the cube solves itself in a few twists that result in a new time of day.  I implemented this on a small 1.44 inch square TFT display with the resolution of 128x128 pixels.  I used a Wemos S2 mini Arduino which uses the capable ESP32 processor.  You will definitely need a fairly fast process for this project.  For instance, an Arduino Uno is too slow for the graphics.

The is an Arduino variant based on my original Rubik's clock that runs Javascript on a Raspberry Pi 4b.  My arduino version is much simpler.  All it does is plays a new GIF video each minute.  The GIFs are generated from taking screenshots of the original Rubik's clock.  Here are some links to the original clock:

- Github: https://github.com/mhirst1960/rubiks-clock
- Instructibles: https://www.instructables.com/Rubiks-Clock/

## What Time is it!

![Alt Text](sdcard_files/12hourclock-128x128/rubiks-clock-0843.gif "The time is 08:43")
![Alt Text](sdcard_files/12hourclock-128x128/rubiks-clock-1002.gif "The time is 10:02")


# Parts

![Alt Text](Images/Rubiks-clock-S2Mini-breadboard.png "Wemos S2 Mini, 1.44 TFT, RTC")

- Wemos S2 mini 
- Adafruit 1.44" Color TFT with Micro SD Socket
- Adafruit PCF8523 Real Time Clock


# Links

## Wemos ESP32 computer

Amazon: https://www.amazon.com/dp/B0B291LZ99

- https://www.wemos.cc/en/latest/s2/s2_mini.html
- https://www.wemos.cc/en/latest/tutorials/s2/get_started_with_arduino_s2.html

## TFT display

Amazon: https://www.amazon.com/dp/B00SK6932C

https://learn.adafruit.com/adafruit-1-44-color-tft-with-micro-sd-socket

## RTC real time clock with a battery

- https://learn.adafruit.com/adafruit-pcf8523-real-time-clock/rtc-with-arduino

## Ardiuno Software

### Libraries to install

- Adafruit Arcada GifDecoder by David Prentice, Craig A. Lindley and Louis Beaudoin
- RTClib by Adafruit

### ESP32 boards

Installation instructions are here:
    https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html

Install esp32 by Espressif Systems
1. Add this link to preferences -> settings -> additional boards manager links:
  https://espressif.github.io/arduino-esp32/package_esp32_index.json

2. Tools -> board -> EPS32 -> LOLIN S2 MINI

## Bugs
- I was never able to robustly get WiFi working on my Wemos S2.  I saw it work only once in all my experiments.  I will experiment more with other Arduino boards.  The Adafruit Feather wFL has good reviews.
- Because of the above issue, I'm not entirely sure if the NPT code will actually update the time after if there is an internet connection.
- I'm not entirely sure that the code is correct for the internal RTC.  Without the External RTC attached you can define ENABLE_RTC_ESP32 to use the internal RTC, but, you would need some sort of minimal battery to hold the time while the board is unplugged.  I haven't tried using a battery to power my Wemos S2. 

## Wiring the TFT

Connect these wires from the Wemos S2 to the Adafruit ST7735 TFT

| Description                  | S2 Mini | Adafruit ST7735 TFT |
| ----                         | ------- | -------------- |
| 3v or 5V power               | 3v3     | Vin 1 |
| 3.3V out                     | -       | 3v3 2 |
| Ground                       | GND     | Gnd 3 |
| SPI clock                    | 7       | SCK 4 |
| MISO (for SD card only)      | 9       | SO 5 |
| MOSI                         | 11      | SI 6 |
| TFT_CS chip select           | 12 (SS) | TCS 7 |
| TFT reset                    | 5       | RST 8 |
| TFT SPI data/command select  | 3       | D/C 9 |
| SD card chip select          | 16      | CCS 10 |
| backlight PWM                | -       | Lite 11 |

## Wiring the Real Time Clock

Connect these wires from the Wemos S2 to the Adafruit pcf8523

| Description        | S2 Mini | Adafruit pcf8523 RTC |
| ----               | ------- | -------------- |
| 3v or 5V power     | 3v3     | VIN 1 |
| Ground             | GND     | GND 2 |
| SCL                | 33      | SCL 3 |
| SDA                | 18      | SDA 4 |
| square wave        | -       | SQW 5 |
