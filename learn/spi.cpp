#include <stdio.h>
#include <stdint.h>

#include "wiringPi.h"
#include "wiringPiSPI.h"

#define CHANNEL 0

int main() {

  wiringPiSPISetup(CHANNEL, 1000);

  uint8_t buffer;

  for( uint8_t i = 0; ; i++ ) {
    buffer = i;
    wiringPiSPIDataRW(CHANNEL, &buffer, 1);
    printf("sent: %d, received: %d\n", i, buffer);
    delay(1000);
  }

  return 0;
}
