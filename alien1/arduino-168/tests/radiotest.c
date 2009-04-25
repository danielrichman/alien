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

/* make -sBj5 radiotest.hex.upload && stty -F /dev/ttyUSB0 cs8 4800 ignbrk -brkint -icrnl -imaxbel -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke noflsh -ixon -crtscts && cat /dev/ttyUSB0 */

payload_message radio_data;
uint8_t i;
uint8_t msg[12] = {'H','e','l','l','o',' ','W','o','r','l','d','\n'};

ISR (TIMER1_COMPA_vect)
{
  radio_proc();
}

uint8_t messages_get_char(payload_message *data, uint8_t message_type)
{
  uint8_t c;

  c = msg[i];
  i++;

  if (i == 12)  i = 0;

  return c;
}

int main(void)
{
       /* Setup IO */
  DDRB  |= _BV(DDB5);     /* Put PB5 as an output (pin13) */

       /* Setup timer to 50hz (from timer1.c) */
  /* Clear the timer counter */
  TCNT1  = 0;

  /* Prescaler will be FCPU/256 (Set bit CS02). 
   * So Timer freq will be 16000000/256 = 62500Hz
   * We want 50Hz; 62500/50 = 1250. So we want an 
   * interrupt every 1250 timer1 ticks. */
  OCR1A   = 1250;

  /* TIMSK1: Enable Compare Match Interrupts (Set bit OCIE1A)*/
  TIMSK1 |= _BV(OCIE1A);
  /* TCCR1B: Clear timer on compare match    (Set bit WGM12) */
  TCCR1B |= _BV(WGM12);
  /* TCCR1B: Prescaler to FCPU/256 & Enable  (Set bit CS12)  */
  TCCR1B |= _BV(CS12);

       /* Turn on interrupts */
  sei();

       /* Light on... */
  PORTB |= _BV(PB5);

       /* Start the radio */
  radio_send();

       /* Sleep */
  for (;;) sleep_mode();
}

