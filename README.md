# Gesture Based Media controller

## What is it
It's a device that controls your TV using IR signals to mimic your TV's remote control. It detects a gesture and then sends the signal that was coded for that gesture.

## The reason I made it
I made it so I can control different media without having to touch anything with my dirty hands while eating.

## Main modules used
- MCU: An ESP32
- Gesture Sensor: PAJ7620
- Display: OLED 0.96" SSD1306 Display

## Pins and their usages
- GPIO4: IR LED
- GPIO15: IR receiver (TL1838)
- GPIO21: SDA (for both the Gesture Sensor and the display)
- GPIO22: SCL (for both the Gesture Sensor and the display)
- GPIO18: delete button (Internally pulled high)

## How to recreate
You can either make the PCB with is [here](./source/pcbway_production/) or connect everything on a breadboard (while using [this](./README.md#pins-and-their-usages) as a reference) then you can flash the software provided [here](./Source/Arduino/Media_Controller_IRTXnRX) to your ESP32.

## Gestures and their functions
- Move Up / Move Down:<br>
Up Button / Down Button
- Move Left / Move Right:<br>
Left Button / Right Button
- Move Forward:<br>
OK button (for TV)
- Circle Clockwise (CW) / Circle Counter-Clockwise (CCW):<br>
Volume Up Button / Volume Down Button
- Wave (shake):<br>
switch between different TV/PC (Inoperative for now, PC functions will be available in V2)

## Demo Video
A demo vid can be watched here:<br>
https://youtu.be/PJmMg9s-Np0

## Notes
- Please note that AI was used to help with **some** parts of the code and not all of it.
- This is the first version of this project and it doesn't have all of the functions. V2 will have battery percentage measurement, better UI, and will be compatible with both TV and PC with the ability to switch between them.
