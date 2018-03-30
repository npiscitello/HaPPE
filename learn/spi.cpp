#include "wiringPi.h"

int main() {

  wiringPiSetup();

  // pin 29 = physical pin 21, don't ask me why
  pinMode(29, OUTPUT);

  while(1) {
    digitalWrite(29, HIGH);
    delay(500);
    digitalWrite(29, LOW);
    delay(500);
  }

  return 0;
}
