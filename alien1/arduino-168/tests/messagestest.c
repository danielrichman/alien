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

/* make -sBj5 messagestest.hex.upload && stty -F /dev/ttyUSB0 cs8 4800 ignbrk -brkint -icrnl -imaxbel -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke noflsh -ixon -crtscts && cat /dev/ttyUSB0 */

uint8_t radio_state = radio_state_not_txing;

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

int main(void)
{
  uint8_t c;

       /* Setup UART */
  UBRR0 = 207;
  UCSR0A = 0;
  UCSR0B = ((_BV(TXEN0))  | (_BV(RXEN0)));
  UCSR0C = ((_BV(UCSZ00)) | (_BV(UCSZ01)));

       /* Spam... */
  for (;;)
  {
    messages_push();

    do
    {
      c = messages_get_char(&radio_data, message_type_radio);
      send_char(c);
    }
    while (c != 0);
  }
}

