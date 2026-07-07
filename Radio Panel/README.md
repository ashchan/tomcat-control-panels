# F-14B Tomcat ARC-159 Radio Panel

AN/ARC-159(V) 1 UHF Control Panel for DCS F-14B Tomcat. Built with Arduino compatible microcontroller board Mega 2560 Pro Mini, use with [DCS-Skunkworks DCS-BIOS fork](https://github.com/DCS-Skunkworks/dcs-bios).

## Parts

* Mega 2560 Pro Mini Board
* TM1637 6-Digit 7-Segment LED
* Mini toggle switches:
  * 4 (on)-off-(on) for frequency dials
  * 1 on-off for SQL
  * 1 (on)-off for READ
* 2 potentiometers for VOL and BRT
* 1 rotary encoder for preset channel selector
* 2 rotary switches for mode selector and function selector
* 2 push buttons for TUNE and LOAD

## Design

This is the final panel (AoA Indexer excluded, for AoA Indexer design see [here](../AoA%20Indexer)).

![arc-159 in DCS](assets/arc-159.jpg)

The size of the original AN/ARC-159(V) is **5.75** inch in width and **4.875** inch in height. I don't need the frequency chart to record preset channel frequencies, so that part is removed from my design to make the panel size smaller:

* width: **5.75** (14.61cm)
* height: **3.75** (9.525cm)

![Panel Design](assets/design.png)

I design the panel using FreeCAD. It consists of three layers. The bottom and middle layers are composed and 3D printed as a single object.

Note the hole diameters are determined by looking at switches, buttons and encoders I have. They are likely to not fit for others.

![Panel CAD](assets/cad.png)

## Mega 2560 Pro Mini PINs Usage

Mega 2560 Pro Mini has enough digital PINs to connect each throw of the (On)-Off-(On) toggle switches and rotary switches directly. The sketch uses `INPUT_PULLUP`, so each switch contact should connect the selected PIN to ground.

This radio control panel has 26 joystick buttons and 2 axes as inputs, and 1 TM1637 LED as output. Note that unlike a Pro Micro, a Mega 2560 does not provide native USB HID by itself; keeping the joystick input behavior requires a Mega-compatible HID path such as MegaJoy, HoodLoader2, or another USB HID bridge.

|  PIN  | ARC-159 Function     | Button/Switch          |  Joystick Button/Axis | Note                      |
|:-----:|----------------------|------------------------|:---------------------:|---------------------------|
|   2   | Chan Sel             | Rotary Encoder 0 CLK   |          BTN8         | Rotate CW                 |
|   3   | Chan Sel             | Rotary Encoder 0 DT    |          BTN9         | Rotate CCW                |
|   4   | Freq LED             | TM1637 LED DIO         |           ❌          | DCS BIOS Output           |
|   5   | Freq LED             | TM1637 LED CLK         |           ❌          | ″                         |
|   6   | Tone                 | Push Button 0          |         BTN10         |                           |
|   7   | Load                 | Push Button 1          |         BTN11         |                           |
|   8   | SQL                  | On-Off Toggle Switch 0 |      BTN12, BTN13     | Off needed as well *1     |
|   9   | Read                 | (On)-Off Switch 0      |         BTN14         |                           |
|   10  | ❌                   | Rotary Encoder 0 SW    |         BTN26         | Misc button *3            |
|   A0  | VOL                  | Potentiometer 0        | Axis0/X, BTN22, BTN23 | BTNs for Rotate CW/CCW    |
|   A1  | BRT                  | Potentiometer 1        | Axis1/Y, BTN24, BTN25 | BTNs for Rotate CW/CCW *2 |
|   22  | Freq 10MHz Switch    | (On)-Off-(On) Switch 0 |          BTN0         | Up contact                |
|   23  | Freq 10MHz Switch    | (On)-Off-(On) Switch 0 |          BTN1         | Down contact              |
|   24  | Freq 1MHz Switch     | (On)-Off-(On) Switch 1 |          BTN2         | Up contact                |
|   25  | Freq 1MHz Switch     | (On)-Off-(On) Switch 1 |          BTN3         | Down contact              |
|   26  | Freq 0.1MHz Switch   | (On)-Off-(On) Switch 2 |          BTN4         | Up contact                |
|   27  | Freq 0.1MHz Switch   | (On)-Off-(On) Switch 2 |          BTN5         | Down contact              |
|   28  | Freq 0.025MHz Switch | (On)-Off-(On) Switch 3 |          BTN6         | Up contact                |
|   29  | Freq 0.025MHz Switch | (On)-Off-(On) Switch 3 |          BTN7         | Down contact              |
|   30  | Mode Selector        | Rotary Switch 0        |         BTN15         | Position 0                |
|   31  | Mode Selector        | Rotary Switch 0        |         BTN16         | Position 1                |
|   32  | Mode Selector        | Rotary Switch 0        |         BTN17         | Position 2                |
|   33  | Function Selector    | Rotary Switch 1        |         BTN18         | Position 0                |
|   34  | Function Selector    | Rotary Switch 1        |         BTN19         | Position 1                |
|   35  | Function Selector    | Rotary Switch 1        |         BTN20         | Position 2                |
|   36  | Function Selector    | Rotary Switch 1        |         BTN21         | Position 3                |

* *1: DCS needs separate keybinds for On and Off. For F-14B ARC-159 Squelch, it has `Squelch SQL`, `Squelch OFF` and `Squelch Toggle`. The first two will be used.
* *2: DCS doesn't have axis binding for `VOL`/`BRT` knobs; Create two buttons for `CW`/`CCW` similar to rotary encoder.
* *3: F-14B Radio Panel doesn't have a center push button on the CHAN SEL knob; Creating one for rotary encoder's button could be useful, say, to reset the panel to initial state.

## DCS-BIOS / HID Hybrid

This radio control panel works in DCS-BIOS/HID Hybrid mode.

1. Inputs: All switches, buttons and rotary encoders are mapped directly in DCS or other simulators. Although this panel is designed and modeled to look like ARC-159,
it can be used anywhere if the Mega 2560 is paired with a USB HID solution that exposes the inputs as a joystick in Windows.
1. Outputs: The TM1637 LED interfaces with DCS-BIOS to display DCS Radio channel or frequency based on mode. When in F-14B Rio seat, it will display the AN/ARC-182 info as well. Currently these modules are supported:
  * F-14B Front Seat
  * F-14B Back Seat (RIO)
  * F-5E

## Arduino Libraries

* [DCS-Skunkworks DCS-BIOS Arduino Library](https://github.com/DCS-Skunkworks/dcs-bios)
* [TM1637TinyDisplay](https://github.com/jasonacox/TM1637TinyDisplay)
* [ArduinoJoystickLibrary](https://github.com/MHeironimus/ArduinoJoystickLibrary)
* [NewEncoder](https://github.com/gfvalvo/NewEncoder)
* [ResponsiveAnalogRead](https://github.com/dxinteractive/ResponsiveAnalogRead)

## License

Available under MIT license.
