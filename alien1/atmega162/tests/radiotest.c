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

#include "../final/radio.c"

/* make -sBj5 radiotest.hex.upload */

payload_message radio_data;

#ifndef TEST_MESSAGE_LONG
  #ifndef TEST_MESSAGE_SHORT
    #ifndef TEST_CHAR
      #define TEST_MESSAGE_SHORT
    #endif
  #endif
#endif

#ifndef TEST_MESSAGE_CHAR
uint8_t i;
#endif
#ifdef TEST_MESSAGE_LONG
uint8_t msg[] = {'H', 'e', 'l', 'l', 'o',  ' ', 'W', 'o', 'r', 'l', 'd', '\n',
                 'a', 'b', 'c', 'd', 'e',  'f', 'g', 'h', 'i', 'j', 'k', 'l', 
                 'm', 'n', 'o', 'p', 'q',  'r', 's', 't', 'u', 'v', 'w', 'x', 
                 'y', 'z', 'A', 'B', 'C',  'D', 'E', 'F', 'G', 'H', 'I', 'J', 
                 'K', 'L', 'M', 'N', 'O',  'P', 'Q', 'R', 'S', 'T', 'U', 'V', 
                 'W', 'X', 'Y', 'Z', '\n', '0', '1', '2', '3', '4', '5', '6', 
                 '7', '8', '9', '\n' };
#endif
#ifdef TEST_MESSAGE_SHORT
uint8_t msg[] = {'H', 'e', 'l', 'l', 'o',  ' ', 'W', 'o', 'r', 'l', 'd', '\n'};
#endif

ISR (TIMER1_COMPA_vect)
{
  radio_proc();
}

uint8_t messages_get_char(payload_message *data)
{
  #ifdef TEST_CHAR
  return 'U';   /* 0b01010101 */
  #else
  uint8_t c;

  c = msg[i];
  i++;

  if (i == sizeof(msg))  i = 0;

  return c;
  #endif
}

int main(void)
{
       /* Setup radio input outputs */
  radio_init();

       /* Setup timer to 50hz (from timer1.c) */
  /* Clear the timer counter */
  TCNT1  = 0;

  /* Prescaler will be FCPU/256 (Set bit CS02). 
   * So Timer freq will be 16000000/256 = 62500Hz
   * We want 50Hz; 62500/50 = 1250. So we want an 
   * interrupt every 1250 timer1 ticks. */
  OCR1A   = 1250;

  /* TIMSK:  Enable Compare Match Interrupts (Set bit OCIE1A)*/
  TIMSK  |= _BV(OCIE1A);
  /* TCCR1B: Clear timer on compare match    (Set bit WGM12) */
  TCCR1B |= _BV(WGM12);
  /* TCCR1B: Prescaler to FCPU/256 & Enable  (Set bit CS12)  */
  TCCR1B |= _BV(CS12);

       /* Turn on interrupts */
  sei();

       /* Start the radio */
  radio_send();

       /* Sleep */
  for (;;) sleep_mode();
}

