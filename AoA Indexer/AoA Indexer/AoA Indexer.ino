#define DCSBIOS_DEFAULT_SERIAL

#include <DcsBios.h>

void setup() {
  DcsBios::setup();
  Serial.begin(9600);
}

DcsBios::LED pltAoaSlow(0x12d4, 0x0004, 2);
DcsBios::LED pltAoaOpt(0x12d4, 0x0008, 3);
DcsBios::LED pltAoaFast(0x12d4, 0x0010, 4);

DcsBios::LED pltHudLightWheels(0x12d6, 0x0008, 5);
DcsBios::LED pltHudLightBrakes(0x12d6, 0x0010, 6);
DcsBios::LED pltHudLightAclsap(0x12d6, 0x0020, 7);
DcsBios::LED pltHudLightNws(0x12d6, 0x0040, 8);
DcsBios::LED pltHudLightAutothr(0x12d6, 0x0080, 9);

void loop() {
  DcsBios::loop();
}
