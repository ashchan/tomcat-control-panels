#define DCSBIOS_DEFAULT_SERIAL

#include <DcsBios.h>
#include <TM1637TinyDisplay6.h>
#include <Joystick.h>
#include <ResponsiveAnalogRead.h>
#include <NewEncoder.h>

#define CHAN_SEL_CLK 0
#define CHAN_SEL_DT 1
#define TONE 2
#define LOAD 3
#define VOL A6
#define VOL_INDEX 4
#define SQL 5
#define BRT A7
#define BRT_INDEX 6
#define READ 7
#define MODE_SEL A8
#define MODE_SEL_INDEX 8
#define FUNC_SEL A9
#define FUNC_SEL_INDEX 9
#define MISC 10
#define LED_DIO 14
#define LED_CLK 15

NewEncoder chanSelEncoder(CHAN_SEL_CLK, CHAN_SEL_DT, -20, 20, 0, FULL_PULSE);
ResponsiveAnalogRead volPot(VOL, true);
ResponsiveAnalogRead brtPot(BRT, true);
TM1637TinyDisplay6 led(LED_CLK, LED_DIO);
Joystick_ joystick(0x07, JOYSTICK_TYPE_JOYSTICK, JOYSTICK_DEFAULT_BUTTON_COUNT,
  0, true, true, false, false, false, false, false, false, false, false, false);

// PINs 0 - 10 state, PIN #1, #2 are not used as CHAN SEL Encoder has more friendly methods
int lastControlState[11] = {-1, -1, LOW, LOW, -1, -1, -1, 0, -1, -1, HIGH};
bool isChanSelTurning = false;
bool isVolTurning = false;
bool isBrtTurning = false;
// A0 - A3, 4 frequency toggle switches
#define FREQ_SWITCH_PIN_1 A0
#define FREQ_SWITCH_STATE_UP    1
#define FREQ_SWITCH_STATE_OFF   2
#define FREQ_SWITCH_STATE_DOWN  3
int lastFreqSwitchState[4] = {FREQ_SWITCH_STATE_OFF, FREQ_SWITCH_STATE_OFF, FREQ_SWITCH_STATE_OFF, FREQ_SWITCH_STATE_OFF};

void handleSimpleButton(int pin, int joystickButton, bool revertValue = false) {
  int state = digitalRead(pin);
  if (state != lastControlState[pin]) {
    joystick.setButton(joystickButton, state == (revertValue ? HIGH : LOW) ? 1 : 0);
    lastControlState[pin] = state;
  }
}

void handleRotarySwitch(int pin, int stateIndex, int positions, int joystickButtonStart) {
  int state = (1023 - analogRead(pin)) / (1024 / positions + 1);
  if (state != lastControlState[stateIndex]) {
    joystick.releaseButton(joystickButtonStart + lastControlState[stateIndex]);
    joystick.pressButton(joystickButtonStart + state);
    lastControlState[stateIndex] = state;
  }
}

int freqSwitchState(int value) {
  if (value < 100) {
    return FREQ_SWITCH_STATE_UP;
  }
  if (value < 900) {
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
// F-5E
int f5FreqSel[5]  = {0, 0, 0, 0, 0};
int f5FreqMode    = 0; // 0/1/2: MANUAL/PRESET/GUARD
int f5Chal        = 1;

void f5ShowFreq() {
  if (f5FreqMode == 0) {
    float value = f5FreqSel[0] * 100 + f5FreqSel[1] * 10 + f5FreqSel[2] + f5FreqSel[3] * 0.1 + f5FreqSel[4] * 0.001;
    led.showNumber(value);
  } else if (f5FreqMode == 1) {
    led.clear();
    led.showNumber(f5Chal, false, f5Chal > 9 ? 2 : 1, f5Chal > 9 ? 3 : 4);
  } else if (f5FreqMode == 2) {
    led.showNumber(243.000);
  }
}

void onF5Uhf100mhzSelChange(unsigned int newValue) {
  f5FreqSel[0] = 4 - newValue;
  f5ShowFreq();
}
DcsBios::IntegerBuffer uhf100mhzSelBuffer(0x76f6, 0x0060, 5, onF5Uhf100mhzSelChange);

void onF5Uhf10mhzSelChange(unsigned int newValue) {
  f5FreqSel[1] = 10 - newValue;
  f5ShowFreq();
}
DcsBios::IntegerBuffer uhf10mhzSelBuffer(0x76f6, 0x0780, 7, onF5Uhf10mhzSelChange);

void onF5Uhf1mhzSelChange(unsigned int newValue) {
  f5FreqSel[2] = 10 - newValue;
  f5ShowFreq();
}
DcsBios::IntegerBuffer uhf1mhzSelBuffer(0x76f6, 0x7800, 11, onF5Uhf1mhzSelChange);

void onF5Uhf01mhzSelChange(unsigned int newValue) {
  f5FreqSel[3] = 10 - newValue;
  f5ShowFreq();
}
DcsBios::IntegerBuffer uhf01mhzSelBuffer(0x76fa, 0x000f, 0, onF5Uhf01mhzSelChange);

void onF5Uhf0025mhzSelChange(unsigned int newValue) {
  f5FreqSel[4] = (4 - newValue) * 25;
  f5ShowFreq();
}
DcsBios::IntegerBuffer uhf0025mhzSelBuffer(0x76fa, 0x0070, 4, onF5Uhf0025mhzSelChange);

void onF5UhfPresetSelChange(unsigned int newValue) {
  f5Chal = newValue + 1;
  f5ShowFreq();
}
DcsBios::IntegerBuffer uhfPresetSelBuffer(0x76f6, 0x001f, 0, onF5UhfPresetSelChange);

void onF5UhfFreqChange(unsigned int newValue) {
  f5FreqMode = newValue;
  f5ShowFreq();
}
DcsBios::IntegerBuffer uhfFreqBuffer(0x768e, 0x6000, 13, onF5UhfFreqChange);

//-----------------------------------------------------------------------------
// Common
void onAcftNameChange(char* newValue) {
  if (String(newValue) == "") {
    led.clear();
  } else {
    // Trigger change to force update display
    int mode = analogRead(MODE_SEL) / (1024 / 3 + 1);
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

  handleSimpleButton(TONE, 10, true);
  handleSimpleButton(LOAD, 11, true);

  if (volPot.hasChanged()) {
    int vol = volPot.getValue();
    joystick.setXAxis(vol);
    int lastPulse = map(lastControlState[VOL_INDEX], 0, 1023, 0, 20);
    int pulse = map(vol, 0, 1023, 0, 20);
    if (pulse > lastPulse) {
      joystick.pressButton(22);
      isVolTurning = true;
    } else if (pulse < lastPulse) {
      joystick.pressButton(23);
      isVolTurning = true;
    }
    lastControlState[VOL_INDEX] = vol;
  } else if (isVolTurning) {
    joystick.releaseButton(22);
    joystick.releaseButton(23);
    isVolTurning = false;
  }

  if (lastControlState[SQL] != digitalRead(SQL)) {
    if (digitalRead(SQL) == LOW) {
      joystick.setButton(12, 1);
      joystick.setButton(13, 0);
    } else {
      joystick.setButton(12, 0);
      joystick.setButton(13, 1);
    }
    lastControlState[SQL] = digitalRead(SQL);
  }

  if (brtPot.hasChanged()) {
    int brt = brtPot.getValue();
    joystick.setYAxis(brt);
    int lastPulse = map(lastControlState[BRT_INDEX], 0, 1023, 0, 20);
    int pulse = map(brt, 0, 1023, 0, 20);
    if (pulse > lastPulse) {
      joystick.pressButton(24);
      isBrtTurning = true;
    } else if (pulse < lastPulse) {
      joystick.pressButton(25);
      isBrtTurning = true;
    }
    lastControlState[BRT_INDEX] = brt;
  } else if (isBrtTurning) {
    joystick.releaseButton(24);
    joystick.releaseButton(25);
    isBrtTurning = false;
  }

  handleSimpleButton(READ, 14);
  handleRotarySwitch(MODE_SEL, MODE_SEL_INDEX, 3, 15);
  handleRotarySwitch(FUNC_SEL, FUNC_SEL_INDEX, 4, 18);

  handleSimpleButton(MISC, 26);

  // A0 - A3
  for (int index = 0; index < 4; index++) {
    int state = freqSwitchState(analogRead(FREQ_SWITCH_PIN_1 + index));
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
