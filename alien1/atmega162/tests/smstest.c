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

/* Simulate messages.c, timer1.c and timer3.c, test sms.c */
#include "../final/sms.c"
payload_message sms_data;

/* make -sBj5 smstest.hex.upload && stty -F /dev/ttyUSB0 cs8 9600 ignbrk -brkint -icrnl -imaxbel -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke noflsh -ixon -crtscts && cat /dev/ttyUSB0 */

volatile uint8_t msg_has_finished;

uint8_t messages_get_char(payload_message *data)
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

ISR (TIMER3_COMPA_vect)
{
  if (sms_mode == sms_mode_waiting)
  {
    sms_mode = sms_mode_ready;

    sms_start();
  }
}

/* Basically just send one message then exit */
int main(void)
{
       /* Initialise Stuff */
  sms_init();
  sms_start();

  /* Note: we don't enable timer3 straight away. */

  OCR3A   = 62500;  /*  1Hz */
  ETIMSK  = _BV(OCIE3A);

  sei();

       /* Sleep */
  for (;;)    sleep_mode();
}

