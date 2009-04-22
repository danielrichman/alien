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

/* TIMER1 is used for many things. It is set up to generate a 50hz interrupt,
 * and each time it does interrupt radio_proc gets a call. Furthermore, every 
 * fifty interrupts ( = one second) the messages.c system gets a call, telling
 * it to distribute a message. Finally, every 60 seconds temperature.c gets
 * called, telling it to start reading temperature. Also, SMSes get distributed
 * by the logic inside the 50hz routine */
/* Also, a LED on Arduino GPIO5 is flashed ;) */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdint.h>
#include <stdlib.h>

#include "gps.h"  
#include "hexdump.h"
#include "messages.h"  
#include "radio.h" 
#include "sms.h"
#include "temperature.h"  
#include "timer1.h"

#define FLASH_LED  PORTD ^= _BV(PD5)

/* We use this to work out when it's the best window of opportunity to nick
 * the UART to send a SMS. gps.c zeros  it whenever it recieves a char,
 * so if it reaches a high value we know that the UART is idle. 
 * See note below */
uint8_t timer1_uart_idle_counter;
uint8_t timer1_want_to_send_sms, timer1_want_to_take_temp;

/* These divide the 50hz into seconds and minutes */
uint8_t timer1_fifty_counter, timer1_second_counter;

/* TODO: Perhaps some sort of watch dog? Check if one of the modules is 
 * TODO: failing (ie. hasn't provided an update in ages) and kick it? */

/* 50hz timer interrupt */
ISR (TIMER1_COMPA_vect)
{
  /* At 50hz we want to trigger the radio. */
  radio_proc();

  /* Increment the counter */
  timer1_fifty_counter++;

  if (timer1_fifty_counter == 50)
  {
    /* One second has passed */
    timer1_fifty_counter = 0;

    /* Something to do each second: */
    messages_push();
    latest_data.system_location.fix_age++;
    latest_data.system_temp.temperature_age++;

    /* Just to remind everyone that we're still alive */
    FLASH_LED;

    /* Increment the other counter */
    timer1_second_counter++;

    if (timer1_second_counter == 60)
    {
      /* Reached the end of the minute */
      timer1_second_counter = 0;

      /* Something to do every minute */
      if (temperature_step == temperature_step_reset_pulse)
      {
        /* If the temperature isn't busy (It shouldn't be!!!) */
        timer1_want_to_take_temp = 1;
      }

      /* TODO: Add SMS want-to-send logic */
      /* if (sms_is_good_idea && timer1_want_to_send_sms == 0)
       *   timer1_want_to_send_sms = 1; */
    }
  }

  /* Try to avoid other interrupts when taking temperature - start it at
   * the end of a 50hz interrupt so it has the biggest chance possible
   * on that score, and use the uart_idle counter to dodge the big
   * gps interrupts. Also, try to avoid gps comms when sending smses. 
   * Sending an sms should only take 1 50hz tick to complete, so 
   * shan't interfere with much else. Just start it a fair distance
   * from gps comms */
  if (timer1_want_to_send_sms == 1 || timer1_want_to_take_temp == 1)
  {
    timer1_uart_idle_counter++;

    if (timer1_uart_idle_counter == 12 && timer1_want_to_take_temp == 1)
    {
      timer1_want_to_take_temp = 0;
      timer1_uart_idle_counter = 0;

      temperature_get();
    }

    if (timer1_uart_idle_counter == 30 && timer1_want_to_send_sms  == 1)
    {
      timer1_want_to_send_sms  = 0;
      timer1_uart_idle_counter = 0;

      /* TODO: Add sms.c and function to take over the UART for sms sending! */
    }
  }
}

void timer1_init()
{
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
  /* TCCR1B: Prescaler to FCPU/256 & Enable  (Set bit CS02)  */
  TCCR1B |= _BV(CS02);

  /* Setup the Flashing LED : Put PD5 as an Output (pin5)    */
  DDRD |= _BV(DDD5);
}

