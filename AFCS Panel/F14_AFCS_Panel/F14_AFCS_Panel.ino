#define DCSBIOS_DEFAULT_SERIAL

#include <DcsBios.h>
#include <AceButton.h>
using namespace ace_button;

#define PIN_PITCH       1
#define PIN_ROLL        0
#define PIN_YAW         2
#define PIN_VEC         3
#define PIN_ALT         4
#define PIN_HDG         5
#define PIN_ENGAGE      6

#define PIN_LED_PITCH   9
#define PIN_LED_ROLL    10
#define PIN_LED_YAW     16
#define PIN_LED_VEC     14
#define PIN_LED_ACL     15
#define PIN_LED_ALT     18
#define PIN_LED_HDG     19
#define PIN_LED_GT      20
#define PIN_LED_ENGAGE  21

int vec = 1;
int hdg = 1;
int engage = 0;

AceButton vecButton(PIN_VEC);
AceButton hdgButton(PIN_HDG);
AceButton engageButton(PIN_ENGAGE);

void vecButtonPressed() {
  // PLT_AUTOPLT_VECTOR_CARRIER, 0..2
  if (vec == 0) {
    sendDcsBiosMessage("PLT_AUTOPLT_VECTOR_CARRIER", "1");
  } else if (vec == 1) {
    sendDcsBiosMessage("PLT_AUTOPLT_VECTOR_CARRIER", "2");
  } else {
    sendDcsBiosMessage("PLT_AUTOPLT_VECTOR_CARRIER", "0");
  }
}

void hdgButtonPressed() {
  // PLT_AUTOPLT_HDG, 0..2
  if (hdg == 0) {
    sendDcsBiosMessage("PLT_AUTOPLT_HDG", "1");
  } else if (hdg == 1) {
    sendDcsBiosMessage("PLT_AUTOPLT_HDG", "2");
  } else {
    sendDcsBiosMessage("PLT_AUTOPLT_HDG", "0");
  }
}

void engageButtonPressed() {
  // PLT_AUTOPLT_ENGAGE, DEC/INC
  sendDcsBiosMessage("PLT_AUTOPLT_ENGAGE", engage == 0 ? "INC" : "DEC");
}

void handleEvent(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  switch (eventType) {
    case AceButton::kEventPressed:
      if (button->getPin() == PIN_VEC) {
        vecButtonPressed();
      }
      if (button->getPin() == PIN_HDG) {
        hdgButtonPressed();
      }
      if (button->getPin() == PIN_ENGAGE) {
        engageButtonPressed();
      }
      break;
  }
}

void updateLED() {
  digitalWrite(PIN_LED_VEC, vec == 2 ? HIGH : LOW);
  digitalWrite(PIN_LED_ACL, vec == 0 ? HIGH : LOW);
  digitalWrite(PIN_LED_HDG, hdg == 2 ? HIGH : LOW);
  digitalWrite(PIN_LED_GT, hdg == 0 ? HIGH : LOW);
}

DcsBios::ActionButton pltAfcsPitchToggle("PLT_AFCS_PITCH", "TOGGLE", PIN_PITCH);
DcsBios::ActionButton pltAfcsRollToggle("PLT_AFCS_ROLL", "TOGGLE", PIN_ROLL);
DcsBios::ActionButton pltAfcsYawToggle("PLT_AFCS_YAW", "TOGGLE", PIN_YAW);
DcsBios::ActionButton pltAutopltAltToggle("PLT_AUTOPLT_ALT", "TOGGLE", PIN_ALT);
DcsBios::LED ledAfcsPitch(0x120a, 0x4000, PIN_LED_PITCH);
DcsBios::LED ledAfcsRoll(0x120a, 0x8000, PIN_LED_ROLL);
DcsBios::LED ledAfcsYaw(0x1210, 0x0001, PIN_LED_YAW);
DcsBios::LED ledAutopltAlt(0x1210, 0x0008, PIN_LED_ALT);
DcsBios::LED ledAutopltEngage(0x1210, 0x0040, PIN_LED_ENGAGE);

void onPltAutopltVectorCarrierChange(unsigned int newValue) {
  vec = newValue;
  updateLED();
}
DcsBios::IntegerBuffer pltAutopltVectorCarrierBuffer(0x1210, 0x0006, 1, onPltAutopltVectorCarrierChange);

void onPltAutopltHdgChange(unsigned int newValue) {
  hdg = newValue;
  updateLED();
}
DcsBios::IntegerBuffer pltAutopltHdgBuffer(0x1210, 0x0030, 4, onPltAutopltHdgChange);

void onPltAutopltEngageChange(unsigned int newValue) {
  engage = newValue;
}
DcsBios::IntegerBuffer pltAutopltEngageBuffer(0x1210, 0x0040, 6, onPltAutopltEngageChange);

void setup() {
  Serial.begin(9600);

  pinMode(PIN_LED_VEC, OUTPUT);
  pinMode(PIN_LED_ACL, OUTPUT);
  pinMode(PIN_LED_HDG, OUTPUT);
  pinMode(PIN_LED_GT, OUTPUT);

  pinMode(PIN_VEC, INPUT_PULLUP);
  pinMode(PIN_HDG, INPUT_PULLUP);
  pinMode(PIN_ENGAGE, INPUT_PULLUP);
  vecButton.setEventHandler(handleEvent);
  hdgButton.setEventHandler(handleEvent);
  engageButton.setEventHandler(handleEvent);

  DcsBios::setup();
}

void loop() {
  DcsBios::loop();

  vecButton.check();
  hdgButton.check();
  engageButton.check();
}
