#include "INITIALIZATION.h"

void initialize_ports() {
   DDRB |= MOTOR;

  // przycisk start/stop
  GICR = (1<<INT0) | (1<<INT1); 
  MCUCR = _BV(ISC01);
  // MCUCR = 0;
  // MCUCR &= _BV(ISC01);
  // MCUCR &= ~_BV(ISC00);
  DDRD &= ~_BV(PD2);
  PORTD |= _BV(PD2);

  // przycisk setup
  // GICR = _BV(INT1);
  // MCUCR = _BV(ISC01);
  DDRD &= ~_BV(PD3);
  PORTD |= _BV(PD3);

// button 1
  DDRD &= ~_BV(PD6);
  PORTD |= _BV(PD6);
  // button 2
  DDRD &= ~_BV(PD7);
  PORTD |= _BV(PD7);
}