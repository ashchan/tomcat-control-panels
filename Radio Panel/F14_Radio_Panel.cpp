#define DCSBIOS_DEFAULT_SERIAL
#define DCSBIOS_DISABLE_SERVO

#include <Arduino.h>
#include <DcsBios.h>
#include <TM1637TinyDisplay6.h>
#include <ResponsiveAnalogRead.h>
#include <NewEncoder.h>

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
  {22, 23},
  {24, 25},
  {26, 27},
  {28, 29},
};

const byte MODE_SEL_PINS[3] = {30, 31, 32};
const byte FUNC_SEL_PINS[4] = {33, 34, 35, 36};
const char* FREQ_DIAL_CONTROLS[4] = {
  "PLT_UHF1_110_DIAL",
  "PLT_UHF1_1_DIAL",
  "PLT_UHF1_01_DIAL",
  "PLT_UHF1_025_DIAL",
};

NewEncoder chanSelEncoder(CHAN_SEL_CLK, CHAN_SEL_DT, -20, 20, 0, FULL_PULSE);
ResponsiveAnalogRead volPot(VOL, true);
ResponsiveAnalogRead brtPot(BRT, true);
TM1637TinyDisplay6 led(LED_CLK, LED_DIO);

int lastToneState = HIGH;
int lastLoadState = HIGH;
int lastSqlState = -1;
int lastReadState = -1;
int lastMiscState = HIGH;
int lastModeState[3] = {HIGH, HIGH, HIGH};
int lastFuncState[4] = {HIGH, HIGH, HIGH, HIGH};

#define FREQ_SWITCH_STATE_UP    1
#define FREQ_SWITCH_STATE_OFF   2
#define FREQ_SWITCH_STATE_DOWN  3
int lastFreqSwitchState[4] = {FREQ_SWITCH_STATE_OFF, FREQ_SWITCH_STATE_OFF, FREQ_SWITCH_STATE_OFF, FREQ_SWITCH_STATE_OFF};

void sendValue(const char* control, unsigned int value) {
  char arg[7];
  utoa(value, arg, 10);
  sendDcsBiosMessage(control, arg);
}

void handleSimpleSwitch(const char* control, int pin, int* lastState, bool revertValue = false) {
  int state = digitalRead(pin);
  if (state != *lastState) {
    sendDcsBiosMessage(control, state == (revertValue ? HIGH : LOW) ? "1" : "0");
    *lastState = state;
  }
}

void handleMultiPositionSwitch(const char* control, const byte pins[], int positions, int lastState[]) {
  for (int index = 0; index < positions; index++) {
    int state = digitalRead(pins[index]);
    if (state != lastState[index]) {
      if (state == LOW) {
        sendValue(control, index);
      }
      lastState[index] = state;
    }
  }
}

int freqSwitchState(int index) {
  if (digitalRead(FREQ_SWITCH_PINS[index][0]) == LOW) {
    return FREQ_SWITCH_STATE_UP;
  }
  if (digitalRead(FREQ_SWITCH_PINS[index][1]) == LOW) {
    return FREQ_SWITCH_STATE_DOWN;
  }
  return FREQ_SWITCH_STATE_OFF;
}

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

  chanSelEncoder.begin();
  pinMode(TONE, INPUT_PULLUP);
  pinMode(LOAD, INPUT_PULLUP);
  pinMode(SQL, INPUT_PULLUP);
  pinMode(READ, INPUT_PULLUP);
  pinMode(MISC, INPUT_PULLUP);
  for (int index = 0; index < 4; index++) {
    pinMode(FREQ_SWITCH_PINS[index][0], INPUT_PULLUP);
    pinMode(FREQ_SWITCH_PINS[index][1], INPUT_PULLUP);
  }
  for (int index = 0; index < 3; index++) {
    pinMode(MODE_SEL_PINS[index], INPUT_PULLUP);
  }
  for (int index = 0; index < 4; index++) {
    pinMode(FUNC_SEL_PINS[index], INPUT_PULLUP);
  }

  led.setBrightness(BRIGHT_3);
  led.clear();
  led.showString("  F-14");
}

unsigned long lastMilli = 0;

void loop() {
  DcsBios::loop();

  if (millis() - lastMilli < 40) {
    return;
  }

  volPot.update();
  brtPot.update();

  if (chanSelEncoder.upClick()) {
    sendDcsBiosMessage("PLT_UHF1_PRESETS", "INC");
  } else if (chanSelEncoder.downClick()) {
    sendDcsBiosMessage("PLT_UHF1_PRESETS", "DEC");
  }

  if (volPot.hasChanged()) {
    sendValue("PLT_UHF1_VOL", map(volPot.getValue(), 0, 1023, 0, 65535));
  }

  if (lastSqlState != digitalRead(SQL)) {
    sendDcsBiosMessage("PLT_UHF1_SQUELCH", digitalRead(SQL) == LOW ? "1" : "0");
    lastSqlState = digitalRead(SQL);
  }

  if (brtPot.hasChanged()) {
    sendValue("PLT_UHF1_BRIGHTNESS", map(brtPot.getValue(), 0, 1023, 0, 65535));
  }

  handleSimpleSwitch("PLT_UHF1_TONE", TONE, &lastToneState, true);
  handleSimpleSwitch("PLT_UHF1_LOAD", LOAD, &lastLoadState, true);
  handleSimpleSwitch("PLT_UHF1_SHOW_PRESET_FREQ", READ, &lastReadState);
  handleMultiPositionSwitch("PLT_UHF1_FREQ_MODE", MODE_SEL_PINS, 3, lastModeState);
  handleMultiPositionSwitch("PLT_UHF1_FUNCTION", FUNC_SEL_PINS, 4, lastFuncState);

  handleSimpleSwitch("PLT_UHF_DISPLAY_TEST", MISC, &lastMiscState);

  for (int index = 0; index < 4; index++) {
    int state = freqSwitchState(index);
    if (state != lastFreqSwitchState[index]) {
      if (state == FREQ_SWITCH_STATE_UP) {
        sendDcsBiosMessage(FREQ_DIAL_CONTROLS[index], "INC");
      } else if (state == FREQ_SWITCH_STATE_DOWN) {
        sendDcsBiosMessage(FREQ_DIAL_CONTROLS[index], "DEC");
      }
      lastFreqSwitchState[index] = state;
    }
  }

  lastMilli = millis();
}
