#define DCSBIOS_DEFAULT_SERIAL

#include <DcsBios.h>
#include <AceButton.h>
using namespace ace_button;

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
#define PIN_SHIFT_SER       14
#define PIN_SHIFT_LATCH     15
#define PIN_SHIFT_CLK       16
#define PIN_LED_TACAN_VEC   20
#define PIN_LED_TACAN_MAN   21
#define PIN_HUD_MODE        A0
#define PIN_STEER_MODE      A1

static const uint8_t NUM_LADDER_BUTTONS = 5;
static const uint16_t LADDER_LEVELS[NUM_LADDER_BUTTONS + 1] = {
  0,
  180,
  480,
  760,
  900,
  1023,
};

static AceButton b0(nullptr, 0);
static AceButton b1(nullptr, 1);
static AceButton b2(nullptr, 2);
static AceButton b3(nullptr, 3);
static AceButton b4(nullptr, 4);
static AceButton* const HUD_MODE_BUTTONS[NUM_LADDER_BUTTONS] = {
  &b0, &b1, &b2, &b3, &b4,
};

static LadderButtonConfig hudModeButtonConfig(
  PIN_HUD_MODE, NUM_LADDER_BUTTONS + 1, LADDER_LEVELS, NUM_LADDER_BUTTONS, HUD_MODE_BUTTONS
);

static AceButton b5(nullptr, 0);
static AceButton b6(nullptr, 1);
static AceButton b7(nullptr, 2);
static AceButton b8(nullptr, 3);
static AceButton b9(nullptr, 4);
static AceButton* const STEER_MODE_BUTTONS[NUM_LADDER_BUTTONS] = {
  &b5, &b6, &b7, &b8, &b9,
};

static LadderButtonConfig steerModeButtonConfig(
  PIN_STEER_MODE, NUM_LADDER_BUTTONS + 1, LADDER_LEVELS, NUM_LADDER_BUTTONS, STEER_MODE_BUTTONS
);

static uint8_t hudMode = 1, steerMode = 2; // 1 - 5

void handleHudModeEvent(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  if (eventType == AceButton::kEventPressed) {
    switchHudMode(button->getPin() + 1);
  }
}

void handleSteerModeEvent(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  if (eventType == AceButton::kEventPressed) {
    switchSteerMode(button->getPin() + 1);
  }
}

void checkButtons() {
  static unsigned long prev = millis();
  unsigned long now = millis();
  if (now - prev > 5) {
    hudModeButtonConfig.checkButtons();
    steerModeButtonConfig.checkButtons();
    prev = now;
  }
}

void updateLED() {
  // Shift register feeds 8 LEDs QA - QH (15, 1-7)
  byte b = (B10000000 >> (hudMode - 1)) | (B100 >> (steerMode - 1));
  digitalWrite(PIN_SHIFT_LATCH, LOW);
  shiftOut(PIN_SHIFT_SER, PIN_SHIFT_CLK, LSBFIRST, b);
  digitalWrite(PIN_SHIFT_LATCH, HIGH);

  // Two more LEDs from Pro Micro
  digitalWrite(PIN_LED_TACAN_VEC, steerMode == 4 ? HIGH : LOW);
  digitalWrite(PIN_LED_TACAN_MAN, steerMode == 5 ? HIGH : LOW);
}

void clearLED() {
  digitalWrite(PIN_SHIFT_LATCH, LOW);
  shiftOut(PIN_SHIFT_SER, PIN_SHIFT_CLK, LSBFIRST, B00000000);
  digitalWrite(PIN_SHIFT_LATCH, HIGH);

  // Two more LEDs from Pro Micro
  digitalWrite(PIN_LED_TACAN_VEC, LOW);
  digitalWrite(PIN_LED_TACAN_MAN, LOW);
}

void switchHudMode(uint8_t mode) {
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
  clearLED();
}
DcsBios::StringBuffer<24> AcftNameBuffer(0x0000, onAcftNameChange);

void setup() {
  pinMode(PIN_HUD_MODE, INPUT);
  pinMode(PIN_STEER_MODE, INPUT);
  pinMode(PIN_SHIFT_SER, OUTPUT);
  pinMode(PIN_SHIFT_LATCH, OUTPUT);
  pinMode(PIN_SHIFT_CLK, OUTPUT);
  pinMode(PIN_LED_TACAN_VEC, OUTPUT);
  pinMode(PIN_LED_TACAN_MAN, OUTPUT);

  hudModeButtonConfig.setEventHandler(handleHudModeEvent);
  hudModeButtonConfig.setFeature(ButtonConfig::kFeatureClick);
  steerModeButtonConfig.setEventHandler(handleSteerModeEvent);
  steerModeButtonConfig.setFeature(ButtonConfig::kFeatureClick);

  DcsBios::setup();
}

void loop() {
  DcsBios::loop();
  checkButtons();
}
