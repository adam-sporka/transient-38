# Transient 38
by AdamJ/MovSD

## Keyboard Layout Cheat Sheet
https://docs.google.com/spreadsheets/d/1aqt8lMjB4ZSdQrAEmFB_FgfocatDTBCdYWFDI8m9jVg/edit?gid=0#gid=0

## MIDI Implementation
* Tip: Switch to PERFORM MODE (S13 + S4, S4 on the original hardware)
* S12 = panic button
* S1, S2, S3 = monophonic mode on voice 1, 2, or 3
* S4 = polyphonic mode
* S8 = unison mode
* MIDI channel <==> Instrument.
  * 1 = bass, 2 == square, 3 == arp, 4 == lead, 5 == bass drum, 6 == snare, 7 == hats, 8 == toms

## How to Update your Unit
Please follow these steps:
* Install Arduino IDE https://www.arduino.cc/en/software/
* Clone this repo
* Open the `transient-38.ino` sketch
* If your unit has a PGM/MIDI switch, make sure to set it to PGM before updating

## Update History
* 2025-06-26 Initial Release
* 2025-08-08 Improved stability and MIDI input

