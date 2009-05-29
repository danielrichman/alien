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

/* make -sBj5 smstest.hex.upload */
/* stty -F /dev/ttyUSB0 cs8 9600 ignbrk -brkint -icrnl -imaxbel -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke noflsh -ixon -crtscts && cat /dev/ttyUSB0 */

volatile uint8_t msg_has_finished;

uint8_t test_message[] = { 'H', 'e', 'l', 'l', 'o', ' ', 'A', 'L', 'I', 'E', 'N', 's', 0x00 };
uint8_t test_message_c;

uint8_t messages_get_char(payload_message *data)
{
  uint8_t c;

  c = test_message[test_message_c];
  test_message_c++;

  return c;
}

void gps_init()
{

}

/* Basically just send one message then exit */
int main(void)
{
       /* Initialise Stuff */
  sms_setup();
  sei();

       /* Sleep */
  for (;;)    sleep_mode();
}

