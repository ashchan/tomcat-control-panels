#define DCSBIOS_DEFAULT_SERIAL

#include <DcsBios.h>
#include <Joystick.h>
#include <ResponsiveAnalogRead.h>

#define PIN_BIT       0
#define PIN_MODE      1
#define PIN_XY        2
#define PIN_FUNC      A0
#define PIN_TENS      A1
#define PIN_ONES      A2
#define PIN_VOL       A3
#define PIN_CRS       A7
#define PIN_CMD       10

#define PIN_LED_CMD_PLT   7
#define PIN_LED_CMD_NFO   8
#define PIN_LED_GO        15
#define PIN_LED_NOGO      16

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
int cmdState = LOW;

unsigned long lastMilli = 0;

Joystick_ joystick(0x08, JOYSTICK_TYPE_JOYSTICK, JOYSTICK_DEFAULT_BUTTON_COUNT,
  0, true, true, false, false, false, false, false, false, false, false, false);
ResponsiveAnalogRead tensPot(PIN_TENS, true);
ResponsiveAnalogRead volPot(PIN_VOL, true);
ResponsiveAnalogRead crsPot(PIN_CRS, true);

DcsBios::LED pltTacanComandPlt(0x12d2, 0x0800, PIN_LED_CMD_PLT);
DcsBios::LED pltTacanComandNfo(0x12d2, 0x1000, PIN_LED_CMD_NFO);
DcsBios::LED pltTacanBit(0x12d4, 0x0020, PIN_LED_GO);
DcsBios::LED pltTacanNogo(0x12d4, 0x0040, PIN_LED_NOGO);

void onAcftNameChange(char* newValue) {
  if (String(newValue) == "") {
    digitalWrite(PIN_LED_CMD_PLT, LOW);
    digitalWrite(PIN_LED_CMD_NFO, LOW);
  } else {
    digitalWrite(PIN_LED_CMD_PLT, HIGH);
  }
}
DcsBios::StringBuffer<24> AcftNameBuffer(0x0000, onAcftNameChange);

void setup() {
  DcsBios::setup();

  pinMode(PIN_BIT, INPUT_PULLUP);
  pinMode(PIN_MODE, INPUT_PULLUP);
  pinMode(PIN_XY, INPUT_PULLUP);
  pinMode(PIN_CMD, INPUT_PULLUP);

  joystick.begin();
}

void loop() {
  DcsBios::loop();

  if (millis() - lastMilli < 40) {
    return;
  }

  tensPot.update();
  volPot.update();
  crsPot.update();

  if (digitalRead(PIN_BIT) != bitState) {
    joystick.setButton(0, bitState == LOW ? 0 : 1);
    bitState = digitalRead(PIN_BIT);
  }

  if (digitalRead(PIN_MODE) != modeState) {
    if (digitalRead(PIN_MODE) == HIGH) {
      sendDcsBiosMessage("PLT_TACAN_MODE_NORMAL_INV", "DEC");
      joystick.setButton(1, 1);
      joystick.setButton(2, 0);
    } else {
      sendDcsBiosMessage("PLT_TACAN_MODE_NORMAL_INV", "INC");
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

  if (tensPot.hasChanged()) {
    // NOTE: this is not generic. Need to adjust based on Pot/Switch type, value range and setup
    int tens = 0;
    int value = analogRead(PIN_TENS);
    if (value < 100) {
      tens = 12;
    } else if (value < 1000) {
      tens = map(1023 - value, 0, 970, 1, 12);
    }

    if (tens != tensState) {
      char arg[2];
      itoa(tens, arg, 10);
      sendDcsBiosMessage("PLT_TACAN_DIAL_TENS", arg);
      if (tens > tensState) {
        joystick.pressButton(10);
      } else {
        joystick.pressButton(11);
      }
      tensState = tens;
      isTensTurning = true;
    }
  } else if (isTensTurning) {
    joystick.releaseButton(10);
    joystick.releaseButton(11);
    isTensTurning = false;
  }

  int ones = (1023 - analogRead(PIN_ONES)) / (1024 / 10 + 1);
  if (ones != onesState) {
    char arg[2];
    itoa(ones, arg, 10);
    sendDcsBiosMessage("PLT_TACAN_DIAL_ONES", arg);
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

  if (volPot.hasChanged()) {
    int vol = volPot.getValue();
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

  if (crsPot.hasChanged()) {
    int crs = crsPot.getValue();
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
  }

  if (digitalRead(PIN_CMD) != cmdState) {
    joystick.setButton(18, cmdState == LOW ? 0 : 1);
    cmdState = digitalRead(PIN_CMD);
  }

  lastMilli = millis();
}
