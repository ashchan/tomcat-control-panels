#define DCSBIOS_DEFAULT_SERIAL

#include <DcsBios.h>

#define PIN_HUD_DECLUTTER   0
#define PIN_HUD_MODE_AWL    1
#define PIN_VDI_MODE_DISP   2
#define PIN_VDI_MODE_LAND   3
#define PIN_HSD_DIS_MODE_A  4
#define PIN_HSD_DIS_MODE_B  5
#define PIN_HSD_ECM_OVER    6
#define PIN_VDI_PW_SW       7
#define PIN_HUD_PW_SW       8
#define PIN_HSD_PW_SW       9
#define PIN_BTN_SHIFT_DATA  10
#define PIN_BTN_SHIFT_CLK   18
#define PIN_BTN_SHIFT_LOAD  19
#define PIN_LED_SHIFT_SER   14
#define PIN_LED_SHIFT_LATCH 15
#define PIN_LED_SHIFT_CLK   16
#define PIN_LED_TACAN_VEC   20
#define PIN_LED_TACAN_MAN   21

uint8_t binaryToMode(uint8_t value) {
  if (value == B11111011) {
    return 2;
  }
  if (value == B11111101) {
    return 3;
  }
  if (value == B11111110) {
    return 4;
  }
  if (value == B01111111) {
    return 5;
  }

  return 1;
}

void checkButtons() {
  static unsigned long prev = millis();
  unsigned long now = millis();
  if (now - prev < 40) {
    return;
  }

  digitalWrite(PIN_BTN_SHIFT_CLK, HIGH);
  digitalWrite(PIN_BTN_SHIFT_LOAD, HIGH);
  uint8_t hud = shiftIn(PIN_BTN_SHIFT_DATA, PIN_BTN_SHIFT_CLK, MSBFIRST);
  uint8_t steer = shiftIn(PIN_BTN_SHIFT_DATA, PIN_BTN_SHIFT_CLK, MSBFIRST);
  digitalWrite(PIN_BTN_SHIFT_LOAD, LOW); 

  if (hud != B11111111) {
    switchHudMode(binaryToMode(hud));
  }
  if (steer != B11111111) {
    switchSteerMode(binaryToMode(steer));
  }

  prev = now;
}

static uint8_t hudMode = 1, steerMode = 2; // 1 - 5

void updateLED() {
  // Shift register feeds 8 LEDs QA - QH (15, 1-7)
  byte b = (B10000000 >> (hudMode - 1)) | (B100 >> (steerMode - 1));
  digitalWrite(PIN_LED_SHIFT_LATCH, LOW);
  shiftOut(PIN_LED_SHIFT_SER, PIN_LED_SHIFT_CLK, LSBFIRST, b);
  digitalWrite(PIN_LED_SHIFT_LATCH, HIGH);

  // Two more LEDs from Pro Micro
  digitalWrite(PIN_LED_TACAN_VEC, steerMode == 4 ? HIGH : LOW);
  digitalWrite(PIN_LED_TACAN_MAN, steerMode == 5 ? HIGH : LOW);
}

void clearLED() {
  digitalWrite(PIN_LED_SHIFT_LATCH, LOW);
  shiftOut(PIN_LED_SHIFT_SER, PIN_LED_SHIFT_CLK, LSBFIRST, B00000000);
  digitalWrite(PIN_LED_SHIFT_LATCH, HIGH);

  // Two more LEDs from Pro Micro
  digitalWrite(PIN_LED_TACAN_VEC, LOW);
  digitalWrite(PIN_LED_TACAN_MAN, LOW);
}

void switchHudMode(uint8_t mode) {
  if (mode == hudMode) {
    return;
  }
  hudMode = mode;
  switch (mode) {
    case 1:
      sendDcsBiosMessage("PLT_HUD_MODE_TAKEOFF", "1");
      break;
    case 2:
      sendDcsBiosMessage("PLT_HUD_MODE_CRUISE", "1");
      break;
    case 3:
      sendDcsBiosMessage("PLT_HUD_MODE_A2A", "1");
      break;
    case 4:
      sendDcsBiosMessage("PLT_HUD_MODE_A2G", "1");
      break;
    case 5:
      sendDcsBiosMessage("PLT_HUD_MODE_LAND", "1");
      break;
  }
}

void switchSteerMode(uint8_t mode) {
  if (mode == steerMode) {
    return;
  }
  steerMode = mode;
  switch (mode) {
    case 1:
      sendDcsBiosMessage("PLT_NAV_STEER_TACAN", "1");
      break;
    case 2:
      sendDcsBiosMessage("PLT_NAV_STEER_DEST", "1");
      break;
    case 3:
      sendDcsBiosMessage("PLT_NAV_STEER_AWL", "1");
      break;
    case 4:
      sendDcsBiosMessage("PLT_NAV_STEER_VECTOR", "1");
      break;
    case 5:
      sendDcsBiosMessage("PLT_NAV_STEER_MAN", "1");
      break;
  }
}

DcsBios::Switch2Pos pltHudDeclutter("PLT_HUD_DECLUTTER", PIN_HUD_DECLUTTER);
DcsBios::Switch2Pos pltHudModeAwl("PLT_HUD_MODE_AWL", PIN_HUD_MODE_AWL);
DcsBios::Switch2Pos pltVdiModeDisp("PLT_VDI_MODE_DISP", PIN_VDI_MODE_DISP);
DcsBios::Switch2Pos pltVdiModeLand("PLT_VDI_MODE_LAND", PIN_VDI_MODE_LAND);
DcsBios::Switch3Pos pltHsdDisMode("PLT_HSD_DIS_MODE", PIN_HSD_DIS_MODE_A, PIN_HSD_DIS_MODE_B);
DcsBios::Switch2Pos pltHsdEcmOver("PLT_HSD_ECM_OVER", PIN_HSD_ECM_OVER);
DcsBios::Switch2Pos pltVdiPwSw("PLT_VDI_PW_SW", PIN_VDI_PW_SW);
DcsBios::Switch2Pos pltHudPwSw("PLT_HUD_PW_SW", PIN_HUD_PW_SW);
DcsBios::Switch2Pos pltHsdPwSw("PLT_HSD_PW_SW", PIN_HSD_PW_SW);

void onPltHudModeChange(char* newValue) {
  hudMode = newValue[0] - '0';
  updateLED();
}
DcsBios::StringBuffer<1> pltHudModeBuffer(0x148a, onPltHudModeChange);

void onPltSteerModeChange(char* newValue) {
  steerMode = newValue[0] - '0';
  updateLED();
}
DcsBios::StringBuffer<1> pltSteerModeBuffer(0x148c, onPltSteerModeChange);

void onAcftNameChange(char* newValue) {
  if (String(newValue) == "") {
    clearLED();
  }
}
DcsBios::StringBuffer<24> AcftNameBuffer(0x0000, onAcftNameChange);

void setup() {
  Serial.begin(9600); 

  pinMode(PIN_BTN_SHIFT_DATA, INPUT);
  pinMode(PIN_BTN_SHIFT_CLK, OUTPUT);
  pinMode(PIN_BTN_SHIFT_LOAD, OUTPUT);
  pinMode(PIN_LED_SHIFT_SER, OUTPUT);
  pinMode(PIN_LED_SHIFT_LATCH, OUTPUT);
  pinMode(PIN_LED_SHIFT_CLK, OUTPUT);
  pinMode(PIN_LED_TACAN_VEC, OUTPUT);
  pinMode(PIN_LED_TACAN_MAN, OUTPUT);

  DcsBios::setup();
}

void loop() {
  DcsBios::loop();
  checkButtons();
}
