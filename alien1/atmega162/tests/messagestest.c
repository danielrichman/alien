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

#include "../final/messages.c"

uint8_t radio_state = radio_state_not_txing;
uint8_t log_state = log_state_reset;  /* messages.c will not call */

void send_char(uint8_t c)
{
  loop_until_bit_is_set(UCSR0A, UDRE0);
  UDR0 = c;
}

void send_char_hd(uint8_t c)
{
  send_char(hexdump_a(c));
  send_char(hexdump_b(c));
}

void radio_send()
{

}

void log_tick()
{

}

int main(void)
{
  uint8_t c;

       /* Setup UART */
  UBRR0L = 207;
  UCSR0B = ((_BV(TXEN0))  | (_BV(RXEN0)));

       /* Spam... */
  for (;;)
  {
    messages_push();

    do
    {
      c = messages_get_char(&radio_data);
      send_char(c);
    }
    while (c != 0);
  }
}

