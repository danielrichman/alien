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
uint8_t timer_counter;

uint8_t test_message[] = { 'H', 'e', 'l', 'l', 'o', ' ', 'A', 'L', 'I', 'E', 'N', 's' };
uint8_t test_message_c;

uint8_t messages_get_char(payload_message *data)
{
  uint8_t c;

  if (test_message_c == sizeof(test_message))
  {
    return 0;
  }

  c = test_message[test_message_c];
  test_message_c++;

  return c;
}

void gps_init()
{

}

ISR (TIMER1_COMPA_vect)
{
  if (sms_waits)
  {
    sms_state++;
    gps_init();
  }
  else if (sms_waitl)
  {
    timer_counter++;

    if (timer_counter == 50)
    {
      sms_state++;
      timer_counter = 0;
    }
  }
  
  if (sms_state == sms_state_end)
  {
    msg_has_finished = 0;
    sms_state = sms_state_null;
  }
}

/* Basically just send one message then exit */
int main(void)
{
       /* Initialise Stuff */
  sms_setup();

  TCNT1  = 0;
  OCR1A   = 1250;

  TIMSK1 |= _BV(OCIE1A);
  TCCR1B |= _BV(WGM12);
  TCCR1B |= _BV(CS12);

  sei();

       /* Sleep */
  for (;;)    sleep_mode();
}

