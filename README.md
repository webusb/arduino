WebUSB ❤ ️Arduino
================

This repository contains an Arduino library for WebUSB-enabling your sketches. Example sketches and JavaScript code are available in the demos directory.

Compatible Hardware
-------------------

WebUSB requires an Arduino model that gives the sketch complete control over the USB hardware. This library has been tested with the following models:

 * Arduino Leonardo
 * Arduino/Genuino Micro

These boards are both based on the ATmega32U4.

Getting Started
---------------

1. Make sure you are running the [latest dev-channel release of Google Chrome](https://www.google.com/chrome/browser/desktop/index.html?extra=devchannel).

2. Make sure the "Experimental Web Platform Features" flag is enabled in chrome://flags. (Sorry, I can't link to it. You need to enter it manually into the omnibox.)

3. Install at least version 1.6.7 of the [Arduino SDK](https://www.arduino.cc/en/Main/Software).

4. The WebUSB library provides all the extra low-level USB code necessary for WebUSB support except for one thing: Your device must be upgraded from USB 2.0 to USB 2.1. To do this go into the SDK installation directory and open `hardware/arduino/avr/cores/arduino/USBCore.h`. Then find the definition of the `D_DEVICE` macro and change `0x200` to `0x210`. That's it!

5. Copy (or symlink) the `library/WebUSB` directory from this repository into the `libraries` folder in your sketchbooks directory.

6. Launch the Arduino SDK. You should see "WebUSB" as an option under "Sketch > Include Library".

7. Load up `demos/rgb/sketch/sketch.ino` and program it to your device.

8. When the sketch is finished uploading you should see a notification from Chrome: "Go to webusb.github.io/arduino/demos/ to connect." Try it out!
