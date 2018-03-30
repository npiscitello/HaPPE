/*  Nick Piscitello
 *  March 2018
 *  Atmel ATMEGA328-PU
 *  fuses: default( l: 0x62, h: 0xD9, e: 0x07)
 *  system clock: 8MHz / 8 = 1MHz
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/twi.h>

// must be 7 bits
#define TWI_ADDR 0x33

volatile uint8_t twi_byte = 0x00;

int main(void) {

  // power on the TWI interface
  PRR &= ~_BV(PRTWI);

  // setup TWI - see pages 279 and 283. Start in SR mode.
  // set the slave address
  TWAR = (TWI_ADDR << 1) | 0x01;
  // enable the TWI (TWEN) with interrupts (TWIE) and 
  // connect it to the bus (TWEA) in slave mode (TWSTA)
  TWCR = 0x00 | _BV(TWEA) | _BV(TWEN) | _BV(TWIE);

  // enable interrupts globally
  sei();

  // FOR DEBUG
  // enable output on pins 14-19
  DDRB |= 0x00 | _BV(DDB0) | _BV(DDB1) | _BV(DDB2) | _BV(DDB3) | _BV(DDB4) | _BV(DDB5);

  // the I2C bus is entirely responsive (slave), so let it wait for stuff
  while(1){};

  // if you get here, something went very wrong
  return 0;
}

// this is called any time the TWI needs attention
ISR(TWI_vect) {
  // figure out what to do based on the TWI status (mask out the prescaler bits)
  switch(TWSR & 0xF8) {
    // Slave Receiver mode
    // intentional fallthrough - our own address (or all call) has been received
    case 0x60:
    case 0x68:
    case 0x70:
    case 0x78:
      // receive data byte, return ACK
      TWCR |= _BV(TWINT) | _BV(TWEA);
      // debug LED
      PORTB |= _BV(DDB0);
      break;

    // Intentional fallthrough - we got data!
    case 0x80:
    case 0x88:
    case 0x90:
    case 0x98:
      // store data byte, return ACK
      twi_byte = TWDR;
      TWCR |= _BV(TWINT) | _BV(TWEA);
      // debug LED
      PORTB |= _BV(DDB1);
      break;

    // stop or repeated start
    case 0xA0:
      // eh, ignore. Make sure we'll be listening.
      TWCR |= _BV(TWINT) | _BV(TWEA);
      // debug LED
      PORTB |= _BV(DDB2);
      break;

    // Slave Transmitter mode
    // intentional fallthrough - we got a data request!
    case 0xA8:
    case 0xB0:
      // push dummy data to the data register
      TWDR = 0xFF;
      // this is the only byte we're sending
      TWCR |= _BV(TWINT);
      TWCR &= ~_BV(TWEA);
      // debug LED
      PORTB |= _BV(DDB3);
      break;

    // intentional fallthrough - these shouldn't occur (non-last data byte transmitted)
    case 0xB8:
    case 0xC0:
      break;

    // last data byte transmitted
    case 0xC8:
      // reset the system
      TWCR |= _BV(TWINT) | _BV(TWEA);
      // debug LED
      PORTB |= _BV(DDB4);
      break;

    // something went really wrong...
    default:
      break;
  }

  // clear the interrupt flag, allowing the TWI hardware to take its next action
  TWCR &= ~_BV(TWINT);
}
