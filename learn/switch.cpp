#include <stdlib.h>
#include <stdio.h>

#include "wiringPi.h"

int main() {

  wiringPiSetup();

  pinMode(12, INPUT);

  while(1) {
    if(digitalRead(12)) {
      // turn relay on
      system("crelay 1 ON");
    } else {
      // turn relay off
      system("crelay 1 OFF");
    }

  }

  return 0;
}
