#define DCSBIOS_DEFAULT_SERIAL
#define DCSBIOS_DISABLE_SERVO

#include <Arduino.h>
#include <DcsBios.h>
#include <TM1637TinyDisplay6.h>
#include <Joystick.h>
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

NewEncoder chanSelEncoder(CHAN_SEL_CLK, CHAN_SEL_DT, -20, 20, 0, FULL_PULSE);
ResponsiveAnalogRead volPot(VOL, true);
ResponsiveAnalogRead brtPot(BRT, true);
TM1637TinyDisplay6 led(LED_CLK, LED_DIO);
Joystick_ joystick(0x07, JOYSTICK_TYPE_JOYSTICK, JOYSTICK_DEFAULT_BUTTON_COUNT,
  0, true, true, false, false, false, false, false, false, false, false, false);

int lastToneState = HIGH;
int lastLoadState = HIGH;
int lastSqlState = -1;
int lastReadState = -1;
int lastMiscState = HIGH;
int lastVolState = -1;
int lastBrtState = -1;
int lastModeState = -1;
int lastFuncState = -1;
bool isChanSelTurning = false;
bool isVolTurning = false;
bool isBrtTurning = false;

#define FREQ_SWITCH_STATE_UP    1
#define FREQ_SWITCH_STATE_OFF   2
#define FREQ_SWITCH_STATE_DOWN  3
int lastFreqSwitchState[4] = {FREQ_SWITCH_STATE_OFF, FREQ_SWITCH_STATE_OFF, FREQ_SWITCH_STATE_OFF, FREQ_SWITCH_STATE_OFF};

void handleSimpleButton(int pin, int joystickButton, int* lastState, bool revertValue = false) {
  int state = digitalRead(pin);
  if (state != *lastState) {
    joystick.setButton(joystickButton, state == (revertValue ? HIGH : LOW) ? 1 : 0);
    *lastState = state;
  }
}

int readDigitalRotarySwitch(const byte pins[], int positions) {
  for (int index = 0; index < positions; index++) {
    if (digitalRead(pins[index]) == LOW) {
      return index;
    }
  }
  return -1;
}

void handleRotarySwitch(const byte pins[], int positions, int joystickButtonStart, int* lastState) {
  int state = readDigitalRotarySwitch(pins, positions);
  if (state != *lastState) {
    if (*lastState >= 0) {
      joystick.releaseButton(joystickButtonStart + *lastState);
    }
    if (state >= 0) {
      joystick.pressButton(joystickButtonStart + state);
    }
    *lastState = state;
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
    int mode = readDigitalRotarySwitch(MODE_SEL_PINS, 3);
    if (mode < 0) {
      mode = 0;
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

  joystick.begin();
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
    joystick.pressButton(8);
    isChanSelTurning = true;
  } else if (chanSelEncoder.downClick()) {
    joystick.pressButton(9);
    isChanSelTurning = true;
  } else if (isChanSelTurning) {
    joystick.releaseButton(8);
    joystick.releaseButton(9);
    isChanSelTurning = false;
  }

  if (volPot.hasChanged()) {
    int vol = volPot.getValue();
    joystick.setXAxis(vol);
    int lastPulse = map(lastVolState, 0, 1023, 0, 20);
    int pulse = map(vol, 0, 1023, 0, 20);
    if (pulse > lastPulse) {
      joystick.pressButton(22);
      isVolTurning = true;
    } else if (pulse < lastPulse) {
      joystick.pressButton(23);
      isVolTurning = true;
    }
    lastVolState = vol;
  } else if (isVolTurning) {
    joystick.releaseButton(22);
    joystick.releaseButton(23);
    isVolTurning = false;
  }

  if (lastSqlState != digitalRead(SQL)) {
    if (digitalRead(SQL) == LOW) {
      joystick.setButton(12, 1);
      joystick.setButton(13, 0);
    } else {
      joystick.setButton(12, 0);
      joystick.setButton(13, 1);
    }
    lastSqlState = digitalRead(SQL);
  }

  if (brtPot.hasChanged()) {
    int brt = brtPot.getValue();
    joystick.setYAxis(brt);
    int lastPulse = map(lastBrtState, 0, 1023, 0, 20);
    int pulse = map(brt, 0, 1023, 0, 20);
    if (pulse > lastPulse) {
      joystick.pressButton(24);
      isBrtTurning = true;
    } else if (pulse < lastPulse) {
      joystick.pressButton(25);
      isBrtTurning = true;
    }
    lastBrtState = brt;
  } else if (isBrtTurning) {
    joystick.releaseButton(24);
    joystick.releaseButton(25);
    isBrtTurning = false;
  }

  handleSimpleButton(TONE, 10, &lastToneState, true);
  handleSimpleButton(LOAD, 11, &lastLoadState, true);
  handleSimpleButton(READ, 14, &lastReadState);
  handleRotarySwitch(MODE_SEL_PINS, 3, 15, &lastModeState);
  handleRotarySwitch(FUNC_SEL_PINS, 4, 18, &lastFuncState);

  handleSimpleButton(MISC, 26, &lastMiscState);

  for (int index = 0; index < 4; index++) {
    int state = freqSwitchState(index);
    if (state != lastFreqSwitchState[index]) {
      int joystickButton = index * 2;
      int joystickValue = 1;
      if (state == FREQ_SWITCH_STATE_OFF) {
        joystickValue = 0;
        if (lastFreqSwitchState[index] == FREQ_SWITCH_STATE_DOWN) {
          joystickButton = index * 2 + 1;
        }
      } else if (state == FREQ_SWITCH_STATE_DOWN) {
        joystickButton = index * 2 + 1;
      }
      joystick.setButton(joystickButton, joystickValue);
      lastFreqSwitchState[index] = state;
    }
  }

  lastMilli = millis();
}
