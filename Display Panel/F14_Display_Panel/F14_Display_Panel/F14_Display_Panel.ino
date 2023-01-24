#include <AceButton.h>
using namespace ace_button;

#define PIN_SHIFT_SER   14
#define PIN_SHIFT_LATCH 15
#define PIN_SHIFT_CLK   16
#define PIN_LED_TACAN_VEC 20
#define PIN_LED_TACAN_MAN 21
#define PIN_DISPLAY_MODE  A0
#define PIN_TACAN_MODE    A1

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
static AceButton* const DISPLAY_MODE_BUTTONS[NUM_LADDER_BUTTONS] = {
  &b0, &b1, &b2, &b3, &b4,
};

static LadderButtonConfig displayModeButtonConfig(
  PIN_DISPLAY_MODE, NUM_LADDER_BUTTONS + 1, LADDER_LEVELS, NUM_LADDER_BUTTONS, DISPLAY_MODE_BUTTONS
);

static AceButton b5(nullptr, 0);
static AceButton b6(nullptr, 1);
static AceButton b7(nullptr, 2);
static AceButton b8(nullptr, 3);
static AceButton b9(nullptr, 4);
static AceButton* const TACAN_MODE_BUTTONS[NUM_LADDER_BUTTONS] = {
  &b5, &b6, &b7, &b8, &b9,
};

static LadderButtonConfig tacanModeButtonConfig(
  PIN_TACAN_MODE, NUM_LADDER_BUTTONS + 1, LADDER_LEVELS, NUM_LADDER_BUTTONS, TACAN_MODE_BUTTONS
);

static uint8_t displayMode = 0, tacanMode = 0;

void handleDisplayModeEvent(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  Serial.print(F("handleEvent(): "));
  Serial.print(F("virtualPin: "));
  Serial.print(button->getPin());
  Serial.print(F("; eventType: "));
  Serial.print(eventType);
  Serial.print(F("; buttonState: "));
  Serial.println(buttonState);

  displayMode = button->getPin();
  setLED();
}

void handleTacanModeEvent(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  Serial.print(F("handleEvent(): "));
  Serial.print(F("virtualPin: "));
  Serial.print(button->getPin());
  Serial.print(F("; eventType: "));
  Serial.print(eventType);
  Serial.print(F("; buttonState: "));
  Serial.println(buttonState);

  tacanMode = button->getPin();
  setLED();
}

void checkButtons() {
  static unsigned long prev = millis();
  unsigned long now = millis();
  if (now - prev > 5) {
    displayModeButtonConfig.checkButtons();
    tacanModeButtonConfig.checkButtons();
    prev = now;
  }
}

void setLED() {
  // Shift register feeds 8 LEDs QA - QH (15, 1-7)
  byte b = (B10000000 >> displayMode) || (B100 >> tacanMode);
  digitalWrite(PIN_SHIFT_LATCH, LOW);
  shiftOut(PIN_SHIFT_SER, PIN_SHIFT_CLK, LSBFIRST, b);
  digitalWrite(PIN_SHIFT_LATCH, HIGH);

  // Two more LEDs from Pro Micro
  writeDigital(PIN_LED_TACAN_VEC, tancanMode == 3 ? HIGH : LOW);
  writeDigital(PIN_LED_TACAN_MAN, tancanMode == 4 ? HIGH : LOW);
}

void setup() {
  Serial.begin(9600);

  pinMode(PIN_DISPLAY_MODE, INPUT);
  pinMode(PIN_TACAN_MODE, INPUT);
  pinMode(PIN_SHIFT_SER, OUTPUT);
  pinMode(PIN_SHIFT_LATCH, OUTPUT);
  pinMode(PIN_SHIFT_CLK, OUTPUT);
  pinMode(PIN_LED_TACAN_VEC, OUTPUT);
  pinMode(PIN_LED_TACAN_MAN, OUTPUT);

  displayModeButtonConfig.setEventHandler(handleDisplayModeEvent);
  displayModeButtonConfig.setFeature(ButtonConfig::kFeatureClick);
  tacanModeButtonConfig.setEventHandler(handleTacanModeEvent);
  tacanModeButtonConfig.setFeature(ButtonConfig::kFeatureClick);
}

void loop() {
  checkButtons();
}
