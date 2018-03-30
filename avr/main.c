/*  Nick Piscitello
 *  March 2018
 *  Atmel ATMEGA328-PU
 *  fuses: default( l: 0x62, h: 0xD9, e: 0x07)
 *  system clock: 8MHz / 8 = 1MHz
 */

#include <avr/io.h>
#include <avr/interrupt.h>

// must be 7 bits
#define TWI_ADDR 0x33

volatile uint8_t spi_byte = 0x00;

int main(void) {

  // power on the TWI interface
  PRR &= ~_BV(PRSPI);

  // MOSI(PB3) = input, MISO(PB4) = output, SCK(PB5) = input, SS(PB2) = input
  DDRB = _BV(DDB4);

  // enable interrupt (SPIE), enable SPI (SPE) in slave mode
  SPCR = _BV(SPIE) | _BV(SPE);

  // enable interrupts globally
  sei();

  // preload data to the SPI bus
  SPDR = 0xFF;

  // the SPI bus is entirely responsive (slave), so let it wait for stuff
  while(1){
    SPDR = spi_byte;
    asm("nop");
    asm("nop");
    asm("nop");
  }

  // if you get here, something went very wrong
  return 0;
}

// This is called when an SPI transaction is done (1 byte has been transferred)
// basically, set up to echo whatever was sent back on the next transaction
ISR(SPI_STC_vect) {
  spi_byte = SPDR;
}
