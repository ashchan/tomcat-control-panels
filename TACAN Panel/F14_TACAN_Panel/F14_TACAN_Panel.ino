#define DCSBIOS_DEFAULT_SERIAL

#include <DcsBios.h>
#include <Joystick.h>

#define PIN_BIT       0
#define PIN_MODE      1
#define PIN_XY        2
#define PIN_FUNC      A0
#define PIN_TENS      A1
#define PIN_ONES      A2
#define PIN_VOL       A3
#define PIN_CRS       A9
#define PIN_CMD       10

#define PIN_LED_GO    15
#define PIN_LED_NOGO  16

const int POT_THRESHOLD = 3;

Joystick_ joystick(0x08, JOYSTICK_TYPE_JOYSTICK, JOYSTICK_DEFAULT_BUTTON_COUNT,
  0, true, true, false, false, false, false, false, false, false, false, false);

int bitState = LOW;
int modeState = -1;
int xyState = -1;
int funcState = -1;
int tensState = -1;
bool isTensTurning = false;
int onesState = -1;
bool isOnesTurning = false;
int volState = -1;
bool isVolTurning = false;
int crsState = -1;
bool isCrsTurning = false;
int cmdState = -1;

void setup() {
  DcsBios::setup();
  Serial.begin(9600);

  pinMode(PIN_BIT, INPUT_PULLUP);
  pinMode(PIN_MODE, INPUT_PULLUP);
  pinMode(PIN_XY, INPUT_PULLUP);
  pinMode(PIN_CMD, INPUT_PULLUP);

  pinMode(PIN_LED_GO, OUTPUT);
  pinMode(PIN_LED_NOGO, OUTPUT);

  joystick.begin();
}

void loop() {
  DcsBios::loop();

  digitalWrite(PIN_LED_GO, HIGH);
  digitalWrite(PIN_LED_NOGO, HIGH);

  if (digitalRead(PIN_BIT) != bitState) {
    joystick.setButton(0, bitState == LOW ? 0 : 1);
    bitState = digitalRead(PIN_BIT);
  }

  if (digitalRead(PIN_MODE) != modeState) {
    if (digitalRead(PIN_MODE) == HIGH) {
      joystick.setButton(1, 1);
      joystick.setButton(2, 0);
    } else {
      joystick.setButton(1, 0);
      joystick.setButton(2, 1);
    }
    modeState = digitalRead(PIN_MODE);
  }

  if (digitalRead(PIN_XY) != xyState) {
    if (digitalRead(PIN_XY) == HIGH) {
      joystick.setButton(3, 1);
      joystick.setButton(4, 0);
    } else {
      joystick.setButton(3, 0);
      joystick.setButton(4, 1);
    }
    xyState = digitalRead(PIN_XY);
  }

  int func = (1023 - analogRead(PIN_FUNC)) / (1024 / 5 + 1);
  if (func != funcState) {
    if (funcState >= 0) {
      joystick.releaseButton(5 + funcState);
    }
    joystick.pressButton(5 + func);
    funcState = func;
  }

  // TODO: handle initial tensState
  int tens = (1023 - analogRead(PIN_TENS)) / (1024 / 13 + 1);
  if (tens != tensState) {
    if (tens > tensState) {
      joystick.pressButton(10);
    } else {
      joystick.pressButton(11);
    }
    tensState = tens;
    isTensTurning = true;
  } else if (isTensTurning) {
    joystick.releaseButton(10);
    joystick.releaseButton(11);
    isTensTurning = false;
  }
// TODO: handle initial onesState
  int ones = (1023 - analogRead(PIN_ONES)) / (1024 / 10 + 1);
  if (ones != onesState) {
    if (ones > onesState) {
      joystick.pressButton(12);
    } else {
      joystick.pressButton(13);
    }
    onesState = ones;
    isOnesTurning = true;
  } else if (isOnesTurning) {
    joystick.releaseButton(12);
    joystick.releaseButton(13);
  }

  int vol = analogRead(PIN_VOL);
  if (abs(vol - volState) > POT_THRESHOLD) {
    joystick.setXAxis(vol);
    int lastPulse = map(volState, 0, 1023, 0, 20);
    int pulse = map(vol, 0, 1023, 0, 20);
    if (pulse > lastPulse) {
      joystick.pressButton(14);
      isVolTurning = true;
    } else if (pulse < lastPulse) {
      joystick.pressButton(15);
      isVolTurning = true;
    }
    volState = vol;
  } else if (isVolTurning) {
    joystick.releaseButton(14);
    joystick.releaseButton(15);
    isVolTurning = false;
  }

/*
  int crs = analogRead(PIN_CRS);
  if (abs(crs - crsState) > POT_THRESHOLD) {
    joystick.setYAxis(crs);
    int lastPulse = map(crsState, 0, 1023, 0, 20);
    int pulse = map(crs, 0, 1023, 0, 20);
    if (pulse > lastPulse) {
      joystick.pressButton(16);
      isCrsTurning = true;
    } else if (pulse < lastPulse) {
      joystick.pressButton(17);
      isCrsTurning = true;
    }
    crsState = crs;
  } else if (isCrsTurning) {
    joystick.releaseButton(16);
    joystick.releaseButton(17);
    isCrsTurning = false;
  }*/

  if (digitalRead(PIN_CMD) != cmdState) {
    if (digitalRead(PIN_CMD) == HIGH) {
      joystick.setButton(18, 1);
      joystick.setButton(19, 0);
    } else {
      joystick.setButton(18, 0);
      joystick.setButton(19, 1);
    }
    cmdState = digitalRead(PIN_CMD);
  }

  delay(40);
}
