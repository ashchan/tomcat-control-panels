# F-14B Tomcat AFCS Panel

## Parts

* 5V/16MHz Pro Micro Board w/ ATmega32U4
* 9 push buttons w/ LED

## Design

* width: **5.75** (14.605cm)
* height: **3.75** (9.525cm)

![afcs in DCS](assets/afcs.jpg)

Switches on the AFCS panel are held to on by solenoids and return to off when disengaged. That needs a lot of magnetic switches but I don't have any.

Instead, LED indicators are added to show current ON switches. Momentary toggle switches are used so they always springloaded to OFF position.

## Arduino Libraries

* Flightpanels [DCS-BIOS Arduino Library](https://github.com/DCSFlightpanels/dcs-bios-arduino-library)
* [AceButton](https://github.com/bxparks/AceButton)

## License

Available under MIT license.
