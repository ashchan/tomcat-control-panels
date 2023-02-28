#define DCSBIOS_DEFAULT_SERIAL

#include <DcsBios.h>

// F-14
DcsBios::LED pltAoaSlow(0x12d4, 0x0004, 2);
DcsBios::LED pltAoaOpt(0x12d4, 0x0008, 3);
DcsBios::LED pltAoaFast(0x12d4, 0x0010, 4);

DcsBios::LED pltHudLightWheels(0x12d6, 0x0008, 5);
DcsBios::LED pltHudLightBrakes(0x12d6, 0x0010, 6);
DcsBios::LED pltHudLightAclsap(0x12d6, 0x0020, 7);
DcsBios::LED pltHudLightNws(0x12d6, 0x0040, 8);
DcsBios::LED pltHudLightAutothr(0x12d6, 0x0080, 9);

// F-16
DcsBios::LED lightAoaUp(0x447a, 0x0800, 2);
DcsBios::LED lightAoaMid(0x447a, 0x1000, 3);
DcsBios::LED lightAoaDn(0x447a, 0x2000, 4);

DcsBios::LED lightArNws(0x447c, 0x0008, 8);

// F-18
DcsBios::LED aoaIndexerHigh(0x7408, 0x0008, 2);
DcsBios::LED aoaIndexerNormal(0x7408, 0x0010, 3);
DcsBios::LED aoaIndexerLow(0x7408, 0x0020, 4);

// F-5E
DcsBios::LED aoaR(0x7602, 0x0100, 2);
DcsBios::LED aoaG(0x7602, 0x0200, 3);
DcsBios::LED aoaY(0x7602, 0x0400, 4);

// Common
void clear() {
  for (int i = 2; i <= 9; ++i) {
    digitalWrite(i, LOW);
  }
}

void onAcftNameChange(char* newValue) {
  clear();
}
DcsBios::StringBuffer<24> AcftNameBuffer(0x0000, onAcftNameChange);

void setup() {
  DcsBios::setup();
}

void loop() {
  DcsBios::loop();
}
