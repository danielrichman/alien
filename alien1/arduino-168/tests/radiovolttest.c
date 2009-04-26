/*
    Copyright (C) 2008  Daniel Richman

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    For a full copy of the GNU General Public License, 
    see <http://www.gnu.org/licenses/>.
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

/* Used to test the generated voltages for the radio */
/* make -sBj5 radiovolttest.hex.upload */

/* From radio.c */
#define RADIO_MARK  PORTD &= ~(_BV(PD3));    /* PD4 on PD3 off */  \
                    PORTD |=   _BV(PD4);
#define RADIO_SPACE PORTD &= ~(_BV(PD4));    /* PD3 on PD4 off */  \
                    PORTD |=   _BV(PD3);

uint8_t i;

ISR (TIMER1_COMPA_vect)
{
  if (i == 0)
  {
    PORTB &= ~_BV(PB5);
    RADIO_SPACE;
    i = 1;
  }
  else
  {
    PORTB |= _BV(PB5);
    RADIO_MARK;
    i = 0;
  }
}

int main(void)
{
       /* Setup IO */
  DDRB  |= _BV(DDB5);     /* Put PB5 as an output (pin13) */

  DDRD  |= _BV(DDD4);     /* Set portD, pin4 as an output.   */
  DDRD  |= _BV(DDD3);     /* Set portD, pin3 as an output.   */
  RADIO_MARK              /* Idle state = mark               */

  /* Prescaler to FCPU/1024; Clear timer on compare match *
   * Timer freq is 16000000/1024 = 15625Hz
   * We want 1Hz, so we want an interrupt every 15625 timer1 ticks. */
  TCCR1B  = (_BV(CS02) | _BV(CS00) | _BV(WGM12));
  OCR1A   = 15625;

       /* Enable Timer1 */
  TCNT1   = 0;            /* Reset timer */
  TIMSK1  = _BV(OCIE1A);  /* Enable Compare Match Interrupts */
  sei();                  /* Turn on interrupts */

       /* Sleep */
  for (;;) sleep_mode();
}

