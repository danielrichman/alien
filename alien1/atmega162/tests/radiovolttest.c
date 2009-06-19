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

#define STATUSLED_RED_ON   PORTA |=   _BV(PA0)
#define STATUSLED_RED_OFF  PORTA &= ~(_BV(PA0))
#define STATUSLED_GRN_ON   PORTA |=   _BV(PA1)
#define STATUSLED_GRN_OFF  PORTA &= ~(_BV(PA1))

uint8_t i;

ISR (TIMER1_COMPA_vect)
{
  if (i == 1)
  {
    /* Mark:  PB0 on PB1 off */
    PORTB &= ~(_BV(PB1));
    PORTB |=   _BV(PB0);
    STATUSLED_RED_ON;
    STATUSLED_GRN_OFF;
    i = 0;
  }
  else 
  {
    /* Space: PB1 on PB0 off */
    PORTB &= ~(_BV(PB0));
    PORTB |=   _BV(PB1);
    STATUSLED_RED_OFF;
    STATUSLED_GRN_ON;
    i = 1;
  }
}

int main(void)
{
  DDRA  |= ((_BV(PA0)) | (_BV(PA1)));

  DDRB  |= _BV(DDB0);     /* Set portB, pin0 as an output.   */
  DDRB  |= _BV(DDB1);     /* Set portB, pin1 as an output.   */

  PORTB &= ~(_BV(PB1));
  PORTB |=   _BV(PB0);
  STATUSLED_RED_ON;
  STATUSLED_GRN_OFF;

  OCR1A   = 62500;
  TCCR1B  = _BV(WGM12)  | _BV(CS12);
  TIMSK   = _BV(OCIE1A);

  TCNT1   = 0;            /* Reset timer */
  TIMSK   = _BV(OCIE1A);  /* Enable Compare Match Interrupts */
  sei();                  /* Turn on interrupts */

  for (;;) sleep_mode();
}

