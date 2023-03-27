#define DCSBIOS_DEFAULT_SERIAL
#define LIBCALL_ENABLEINTERRUPT

#include <DcsBios.h>
#include <EncoderPCI.h>
#include <LedControl.h>

#define MODE_MANUAL   1
#define MODE_GUARD    2
#define MODE_PRESET   3
#define MODE_NULL     -1

Encoder dial110(1, 0);
Encoder dial1(2, 3);
Encoder dial01(7, 8);
Encoder dial025(9, 10);
Encoder chan(14, 15);

// 8-digit, only the middle 6 are used.
LedControl led = LedControl(4, 5, 16, 1);

void onF14PltVuhfRemoteDispChange(char* value) {
  led.clearDisplay(0);
  if (value[3] == '.') {
    led.setDigit(0, 6, value[0] - '0', false);
    led.setDigit(0, 5, value[1] - '0', false);
    led.setDigit(0, 4, value[2] - '0', true);
    led.setDigit(0, 3, value[4] - '0', false);
    led.setDigit(0, 2, value[5] - '0', false);
    led.setDigit(0, 1, value[6] - '0', false);
  } else {
    int chal = String(value[3]).toInt() * 10 + value[4] - '0';
    if (chal > 9) {
      led.setDigit(0, 3, value[3] - '0', false);
      led.setDigit(0, 2, value[4] - '0', false);
    } else {
      led.setDigit(0, 2, value[4] - '0', false);
    }
  }
}
DcsBios::StringBuffer<7> pltVuhfRemoteDispBuffer(0x1482, onF14PltVuhfRemoteDispChange);

void onAcftNameChange(char* newValue) {
  if (String(newValue) == "") {
    led.clearDisplay(0);
  }
}
DcsBios::StringBuffer<24> AcftNameBuffer(0x0000, onAcftNameChange);

long val110 = 0;
long val1 = 0;
long val01 = 0;
long val025 = 0;
long valChan = 0;
long valMode = MODE_NULL;
unsigned long lastMilli = 0;

void setup() {
  DcsBios::setup();

  led.shutdown(0, false);
  led.setIntensity(0, 8);
  led.clearDisplay(0);

  pinMode(19, INPUT_PULLUP);
  pinMode(20, INPUT_PULLUP);
  pinMode(21, INPUT_PULLUP);
}

void loop() {
  DcsBios::loop();

  if (millis() - lastMilli < 50) {
    return;
  }

  long manual = digitalRead(19);
  long guard = digitalRead(20);
  long preset = digitalRead(21);

  long mode = MODE_PRESET;
  if (manual == LOW) {
    mode = MODE_MANUAL;
  } else if (guard == LOW) {
    mode = MODE_GUARD;
  }
  if (mode != valMode) {
    char arg[2];
    itoa(mode, arg, 10);
    sendDcsBiosMessage("RIO_VUHF_FREQ_MODE", arg);
    valMode = mode;
  }

  int32_t value110 = dial110.read() >> 2;
  if (value110 != val110) {
    sendDcsBiosMessage("RIO_VUHF_110_DIAL", value110 > val110 ? "2" : "0");
    sendDcsBiosMessage("RIO_VUHF_110_DIAL", "1");
    val110 = value110;
  }

  long value1 = dial1.read() >> 2;
  if (value1 != val1) {
    sendDcsBiosMessage("RIO_VUHF_1_DIAL", value1 > val1 ? "2" : "0");
    sendDcsBiosMessage("RIO_VUHF_1_DIAL", "1");
    val1 = value1;
  }

  long value01 = dial01.read() >> 2;
  if (value01 != val01) {
    sendDcsBiosMessage("RIO_VUHF_01_DIAL", value01 > val01 ? "2" : "0");
    sendDcsBiosMessage("RIO_VUHF_01_DIAL", "1");
    val01 = value01;
  }

  long value025 = dial025.read() >> 2;
  if (value025 != val025) {
    sendDcsBiosMessage("RIO_VUHF_025_DIAL", value025 > val025 ? "2" : "0");
    sendDcsBiosMessage("RIO_VUHF_025_DIAL", "1");
    val025 = value025;
  }

  long valueChan = chan.read() >> 2;
  if (valueChan != valChan) {
    sendDcsBiosMessage("RIO_VUHF_PRESETS", valueChan > valChan ? "INC" : "DEC");
    valChan = valueChan;
  }

  lastMilli = millis();
}