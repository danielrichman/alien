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

/* The arduino bootloader turns on the UART. Disable it here so we can
 * directly access the FTDI chip without the ATMEGA interfering. We used this
 * to test the GPS. */

int main(void)
{
       /* Setup IO */
  DDRB  |= _BV(DDB5);     /* Put PB5 as an output (pin13) */
  PORTB |= _BV(PB5);      /* Turn it on */

       /* Disable UART so it can be passed-through */
  UCSR0B = 0;

       /* Sleep */
  for (;;) sleep_mode();
}
