# Display current note names on four 8x8 led dot-matrix displays

This is an Arduino project, to try to help my band mates and show where we are in the song.

## Features:
* Show MIDI note names
* two-color LED indicate whether we are Playing back in the DAW (or recording! -> TODO)
* show beat according to current tempo (MIDI_CLOCK)

Included is a case for printing on any 3D printer including the OpenSCAD source file.

## Requirements:
* I've used an Arduino Pro Micro, which can do MIDI over USB
* MAX7219 based dot-matrix display module (see parts list below)
* LedControl library from https://www.electronoobs.com/ledcontrol.php
* UsbMidi library from https://github.com/BlokasLabs/usbmidi


## Parts
* Arduino Pro Micro (https://www.makershop.de/plattformen/arduino/pro-micro-atmega-32u4/)
* LED Dot-matrix display module (MAX7219) (https://www.makershop.de/display/led-matrix/dot-matrix-modul-8x8-max7219/)
* One or two LEDs and matching resistors 
* Micro-USB cable
