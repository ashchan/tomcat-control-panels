#define DCSBIOS_DEFAULT_SERIAL
#define DCSBIOS_DISABLE_SERVO

#include <Arduino.h>
#include <DcsBios.h>
#include <TM1637TinyDisplay6.h>

#define CHAN_SEL_CLK 2
#define CHAN_SEL_DT 3
#define LED_DIO 4
#define LED_CLK 5
#define TONE 6
#define LOAD 7
#define SQL 8
#define READ 9
#define MISC 10
#define VOL A0
#define BRT A1

const byte FREQ_SWITCH_PINS[4][2] = {
  {11, 12},
  {13, 14},
  {15, 16},
  {17, 18},
};

const byte MODE_SEL_PINS[3] = {19, 20, 21};
const byte FUNC_SEL_PINS[4] = {22, 23, 24, 25};

typedef DcsBios::PotentiometerEWMA<5, 256, 5> PanelPotentiometer;

DcsBios::RotaryEncoder chanSelEncoder("PLT_UHF1_PRESETS", "INC", "DEC", CHAN_SEL_CLK, CHAN_SEL_DT);
DcsBios::Switch2Pos toneSwitch("PLT_UHF1_TONE", TONE);
DcsBios::Switch2Pos loadSwitch("PLT_UHF1_LOAD", LOAD);
DcsBios::Switch2Pos squelchSwitch("PLT_UHF1_SQUELCH", SQL);
DcsBios::Switch2Pos readSwitch("PLT_UHF1_SHOW_PRESET_FREQ", READ);
DcsBios::Switch2Pos displayTestSwitch("PLT_UHF_DISPLAY_TEST", MISC);
PanelPotentiometer volPot("PLT_UHF1_VOL", VOL, true);
PanelPotentiometer brtPot("PLT_UHF1_BRIGHTNESS", BRT, true);
DcsBios::SwitchMultiPos modeSelector("PLT_UHF1_FREQ_MODE", MODE_SEL_PINS, 3);
DcsBios::SwitchMultiPos functionSelector("PLT_UHF1_FUNCTION", FUNC_SEL_PINS, 4);
DcsBios::ActionButton freq10MhzInc("PLT_UHF1_110_DIAL", "INC", FREQ_SWITCH_PINS[0][0]);
DcsBios::ActionButton freq10MhzDec("PLT_UHF1_110_DIAL", "DEC", FREQ_SWITCH_PINS[0][1]);
DcsBios::ActionButton freq1MhzInc("PLT_UHF1_1_DIAL", "INC", FREQ_SWITCH_PINS[1][0]);
DcsBios::ActionButton freq1MhzDec("PLT_UHF1_1_DIAL", "DEC", FREQ_SWITCH_PINS[1][1]);
DcsBios::ActionButton freq01MhzInc("PLT_UHF1_01_DIAL", "INC", FREQ_SWITCH_PINS[2][0]);
DcsBios::ActionButton freq01MhzDec("PLT_UHF1_01_DIAL", "DEC", FREQ_SWITCH_PINS[2][1]);
DcsBios::ActionButton freq025MhzInc("PLT_UHF1_025_DIAL", "INC", FREQ_SWITCH_PINS[3][0]);
DcsBios::ActionButton freq025MhzDec("PLT_UHF1_025_DIAL", "DEC", FREQ_SWITCH_PINS[3][1]);

TM1637TinyDisplay6 led(LED_CLK, LED_DIO);

//-----------------------------------------------------------------------------
// F-14B
void f14ShowFreq(char* value) {
  if (value[3] == '.') {
    float preq = (value[0] - '0') * 100 + (value[1] - '0') * 10 + (value[2] - '0') + (float)(value[4] - '0') * 0.1 + (float)(value[5] - '0') * 0.01 + (float)(value[6] - '0') * 0.001;
    led.showNumber(preq);
  } else {
    int chal = String(value[3]).toInt() * 10 + value[4] - '0';
    led.clear();
    led.showNumber(chal, false, chal > 9 ? 2 : 1, chal > 9 ? 3 : 4);
  }
}

void onF14PltUhf1BrightnessChange(unsigned int newValue) {
  int brightness = map(newValue, 0, 65535, BRIGHT_0 - 1, BRIGHT_7);
  led.setBrightness(brightness, brightness >= BRIGHT_0);
}
DcsBios::IntegerBuffer pltUhf1BrightnessBuffer(0x1246, 0xffff, 0, onF14PltUhf1BrightnessChange);

void onF14PltUhfRemoteDispChange(char* newValue) {
  f14ShowFreq(newValue);
}
DcsBios::StringBuffer<7> pltUhfRemoteDispBuffer(0x1472, onF14PltUhfRemoteDispChange);

//-----------------------------------------------------------------------------
// Common
void onAcftNameChange(char* newValue) {
  if (String(newValue) == "") {
    led.clear();
  } else {
    // Trigger change to force update display
    int mode = 0;
    for (int index = 0; index < 3; index++) {
      if (digitalRead(MODE_SEL_PINS[index]) == LOW) {
        mode = index;
        break;
      }
    }
    char arg[2];
    itoa((mode + 1) % 3, arg, 10);
    sendDcsBiosMessage("PLT_UHF1_FREQ_MODE", arg);
    delay(200);
    itoa(mode, arg, 10);
    sendDcsBiosMessage("PLT_UHF1_FREQ_MODE", arg);
  }
}
DcsBios::StringBuffer<24> AcftNameBuffer(0x0000, onAcftNameChange);

//-----------------------------------------------------------------------------
// Main loop
void setup() {
  DcsBios::setup();

  led.setBrightness(BRIGHT_3);
  led.clear();
  led.showString("  F-14");
}

void loop() {
  DcsBios::loop();
}
