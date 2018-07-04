# irsniff
The repository contains an Arduino project to read and report infrared laser tag packets for [Recoil](https://www.theworldisnowgame.com/) laser tag toys made by [Skyrocket, LLC](https://www.skyrocketon.com/).

## Setup

To use this sketch, you will need to attach a 38kHz infrared sensor to your Arduino.  A [TSOP34838](https://www.vishay.com/docs/82489/tsop322.pdf) was used to test, and is likely more similar to the receivers used in the Recoil guns, but most 38kHz infrared sensors should work, including [this one from Adafruit](https://www.adafruit.com/product/157).

The sensor has three pins: ground, power, and signal.  Connect ground to any of the Arduino pins labeled "ground" or "gnd", the power pin to "3.3V" or "5V", and the signal pin to any of the GPIO pins (the sketch assumes pin 7, but this can be easily changed).

Optionally, connect one or two LEDs to two additional GPIO pins for a visual indication of infrared transmissions.  [These](https://www.ebay.com/itm/202036030804) are convenient for this purpose.  The sketch assumes pins 50 and 52, but this can be easily changed.

## How to use

Upload the sketch and connect to the Arduino with the serial monitor at 115200 baud.  After observing "IRSniff ready", shoot a Recoil gun or interact with a Recoil grenade with the barrel or grenade pointed roughly at the IR sensor.  Each unique packet should print a message to the serial monitor.  Duplicate packets are not printed, but they do flash the LEDs (if attached).

## Packet format

See [this repository's wiki](https://github.com/infratag/irsniff/wiki/Infrared-packet-format) for a description of the infrared packet format.

