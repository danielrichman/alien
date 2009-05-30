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
#define RADIO_MARK  PORTB &= ~(_BV(PD1));    /* PB0 on PB1 off */  \
                    PORTB |=   _BV(PD0);
#define RADIO_SPACE PORTB &= ~(_BV(PD0));    /* PB1 on PB0 off */  \
                    PORTB |=   _BV(PD1);

uint8_t i;

ISR (TIMER1_COMPA_vect)
{
  if (i == 0)
  {
    RADIO_SPACE;
    i = 1;
  }
  else
  {
    RADIO_MARK;
    i = 0;
  }
}

int main(void)
{
       /* Setup IO */
  DDRB  |= _BV(DDB0);     /* Set portB, pin0 as an output.   */
  DDRB  |= _BV(DDB1);     /* Set portB, pin1 as an output.   */
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

