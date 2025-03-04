# Arduino Rubik's Clock

The Arduino Rubik's Clock is a clock that shows the time of day on the face of a Rubik's Cube.  Every minute of the day the cube solves itself in a few twists that result in a new time of day.  I implemented this on a small 1.44 inch square TFT display with the resolution of 128x128 pixels.  I used a Wemos S2 mini Arduino which uses the capable ESP32 processor.  You will definitely need a fairly fast process for this project.  For instance, an Arduino Uno is too slow for the graphics.

The is an Arduino variant based on my original Rubik's clock that runs Javascript on a Raspberry Pi 4b.  My arduino version is much simpler.  All it does is plays a new GIF video each minute.  The GIFs are generated from taking screenshots of the original Rubik's clock.  Here are some links to the original clock:

- Github: https://github.com/mhirst1960/rubiks-clock
- Instructibles: https://www.instructables.com/Rubiks-Clock/

## What Time is it!

![Alt Text](sdcard_files/12hourclock-128x128/rubiks-clock-0843.gif "The time is 08:43")
![Alt Text](sdcard_files/12hourclock-128x128/rubiks-clock-1002.gif "The time is 10:02")

![Alt Text](images/rubiks-clock-1054.gif "The time is 10:54")

# Parts

![Alt Text](images/Rubiks-clock-S2Mini-breadboard.png "Wemos S2 Mini, 1.44 TFT, RTC")

- Wemos S2 mini 
- Adafruit 1.44" Color TFT with Micro SD Socket
- Adafruit PCF8523 Real Time Clock
- SD Card at least 4GB and no larger than 32GB


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

## Arduino Software

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

2. Tools -> board -> EPS32 -> Adafruit Feather S2


## Bugs
- WiFi on the Wemos S2 seems to only work when I select a 2.4GHz network.
- The implementation I have requires an external RTC.  To hold the time in the internal RTC you still need a Lithium battery for when transporting and as a wearable.  I tried using an Adafruit battery and charger which works great until the battery runs low.  Recovering after the charge Is unreliable and more often than not I found I needed to toggle power connection to the Wemos.  There for RTC time was lost. 

## Wiring the S2 mini and TFT

Connect these wires from the Adafruit ST7735 TFT to either of these:
- Wemos S2
  -  https://www.wemos.cc/en/latest/s2/s2_mini.html
- Adafruit Feather ESP32-S2 (product 5000 or 5303)
  - Feather: https://www.adafruit.com/product/5000
  - https://learn.adafruit.com/adafruit-esp32-s2-feather/pinouts 

| Description                  | ST7735 TFT | S2 Mini | 
| ----                         | -------    | --------|
| 3v or 5V power               | Vin 1      | 3v3     |
| 3.3V out                     | 3v3 2      | -       |
| Ground                       | Gnd 3      | GND     |
| SPI clock                    | SCK 4      | 7       |
| MISO (for SD card only)      | SO 5       | 9       |
| MOSI                         | SI 6       | 11      |
| TFT_CS chip select           | TCS 7      | 39      |
| TFT reset                    | RST 8      | 5       |
| TFT SPI data/command select  | D/C 9      | 3       |
| SD card chip select          | CCS 10     | 16      |
| backlight PWM                | Lite 11    | 33       |


## Wiring the Real Time Clock

Connect these wires from the Wemos S2 to the Adafruit pcf8523

| Description        | S2 Mini | Adafruit pcf8523 RTC |
| ----               | ------- | -------------- |
| 3v or 5V power     | 3v3     | VIN 1 |
| Ground             | GND     | GND 2 |
| SCL                | 33      | SCL 3 |
| SDA                | 35      | SDA 4 |
| square wave        | -       | SQW 5 |

## Clock Configuration

You can set your WiFi password and timezone by editing a couple of files found ont he SD Card.

### Timezone

Set your timezone by editing time.conf at the top level of the SD Card (This is sdcard_files/time.conf in the github repo, which gets copied to the SD Card during installation).  There are various options documented as comments in that file but typically you just need to set the "TZ=" variable.

Modify this line:

```
TZ=America/New_York
```

to a value that represents where you live.  The list of possible values is extensive and can be found in zones.csv or https://github.com/nayarsystems/posix_tz_db/blob/master/zones.json

### WiFi Password

Set you WiFi connection by editing wifi.conf also found at the top level of the SD Card.  Change these two lines to match WiFi where you will be using your clock:

```
ssid=mynetwork
password=mywifipassword
```

So, for example, if your network name is "xfinitywifi" and your password is "wifipassword1234" then change those lines to:

```
ssid=xfinitywifi
password=wifipassword1234
```

#### up to 10 SSIDs

Also, if you want to configure more than one SSID, you can add up to 10 pairs of ssid/password.  Just add more values to the end of wifi.conf:

```
ssid=myfriendswifi
password=friendpassword
```

When your Rubik's clock powers on it will connect to your WiFi using the credentials.  If the first one fails, it will try the second, and so on until one of them works.


#### 2.4 GHz is best
I have two choices at my house.  I have the best luck when I use the 2.4 GHz connection.

## 24 hour vs. AM/PM

By default the clock shows time as AM/PM.  At one hour after noon, the clock shows 1:00.  If you prefer for it to show 13:00 then you will need to edit rubiks_clock_128x128_S2mini.ino.  Find these lines near the top:

```
#define CLOCK12
//#define CLOCK24
```

And change it to this:

```
//#define CLOCK12
#define CLOCK24
```

(TODO: maybe someday I will add some code so that can be configured in time.conf)
