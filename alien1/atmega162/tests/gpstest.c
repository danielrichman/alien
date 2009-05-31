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

/* Simulate radio.c and add debugging hacks, test gps.c and messages.c */
#define ALIEN_DEBUG_GPS
#include "../final/gps.c"
#include "../final/messages.c"
uint8_t log_state = log_state_null;
uint8_t timers_uart_idle_counter;
uint8_t radio_state = radio_state_not_txing;

/* make -sBj5 gpstest.hex.upload && stty -F /dev/ttyUSB0 cs8 4800 ignbrk -brkint -icrnl -imaxbel -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke noflsh -ixon -crtscts && cat /dev/ttyUSB0 */

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

void log_start()
{

}

ISR (TIMER1_COMPA_vect)
{
  uint8_t i, c;

  timers_uart_idle_counter++;

  if (timers_uart_idle_counter == 26)
  {
    /* minus three is the last three bytes, which is binary data */
    for (i = 0; i < sizeof(latest_data.system_location) - 3; i++)
    {
      c = ba(latest_data.system_location)[i];
      if (c == 0)  c = ' ';
      send_char(c);
    }

    send_char_hd(latest_data.system_location.flags);

    /* and again, for gps_data */
    for (i = 0; i < sizeof(gps_data) - 3; i++)
    {
      c = ba(gps_data)[i];
      if (c == 0)  c = ' ';
      send_char(c);
    }

    send_char_hd(gps_data.flags);



    send_char('\n');

    messages_push();

    do
    {
      c = messages_get_char(&radio_data);
      send_char(c);
    }
    while (c != 0);
   

    PORTC ^= _BV(PC0);
  }
}

int main(void)
{
       /* Setup IO */
  DDRC  |= _BV(DDC0);     /* Put PC0 as an output (pin21) */

       /* Initialise Stuff */
  gps_init();
  UCSR0B |= _BV(TXEN0);

       /* Light on... */
  PORTC |= _BV(PC0);

       /* Setup & Enable TIMER1 at 50hz */
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

       /* Go go go! - Inable Interrupts */
  sei();

       /* Sleep */
  for (;;) sleep_mode();
}

