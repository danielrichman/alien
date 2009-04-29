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
#include <util/delay.h>

/* Simulate messages.c and timer1.c, test sms.c */
#include "../final/sms.c"
payload_message sms_data;

/* make -sBj5 smstest.hex.upload && stty -F /dev/ttyUSB0 cs8 9600 ignbrk -brkint -icrnl -imaxbel -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke noflsh -ixon -crtscts && (sleep 4; echo hellohello > /dev/ttyUSB0) & cat /dev/ttyUSB0 */

volatile uint8_t msg_has_finished;

uint8_t messages_get_char(payload_message *data, uint8_t type)
{
  uint8_t c;

  if (msg_has_finished)
  {
    return 0;
  }

  loop_until_bit_is_set(UCSR0A, RXC0);
  c = UDR0;

  if (c == '\n')
  {
    /* give sms.c the \n then send nulls */
    msg_has_finished = 1;
  }

  return c;
}

void gps_init()
{

}

ISR (TIMER1_COMPA_vect)
{
  if (sms_state == sms_state_wait)
  {
    sms_state++;
  }
  else if (sms_state == sms_state_end)
  {
    msg_has_finished = 0;
    sms_state = sms_state_null;
    gps_init();

    sms_setup();
    UCSR0B |= _BV(RXEN0);
  }
}

int main(void)
{
       /* Initialise Stuff */
  sms_setup();

  UCSR0B |= _BV(RXEN0);

  TCNT1  = 0;
  OCR1A   = 1250;

  TIMSK1 |= _BV(OCIE1A);
  TCCR1B |= _BV(WGM12);
  TCCR1B |= _BV(CS12);

  sei();

       /* Sleep */
  for (;;)    sleep_mode();
}

