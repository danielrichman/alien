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

#include "camera.h"
#include "gps.h"  
#include "hexdump.h"
#include "main.h"
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
uint8_t timer1_uart_idle_counter, timer1_want_to_send_sms;

/* These divide the 50hz into seconds and minutes */
uint8_t timer1_fifty_counter, timer1_second_counter;
uint8_t timer1_temperature_counter;

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

    /* Somethings to do each second: */
    messages_push();                           /* Push Messages */
    latest_data.system_location.fix_age++;     /* Increment Age */

    /* Set the age bits on the temperature values */
    latest_data.system_temp.external_temperature |= temperature_ubits_age;
    latest_data.system_temp.internal_temperature |= temperature_ubits_age;

    /* Just to remind everyone that we're still alive */
    FLASH_LED;

    /* Increment the other counter */
    timer1_second_counter++;

    if (timer1_second_counter == 60)
    {
      /* Reached the end of the minute */
      timer1_second_counter = 0;

      /* Something to do every minute */
      temperature_state = temperature_state_want_to_get;

      /* TODO: Add SMS want-to-send logic */
      /* if (sms_is_good_idea)
       *   timer1_want_to_send_sms = 1; */
    }
  }

  /* Deal with temperature wait-loop */
  if ( temperature_state == temperature_state_requested &&
       (timer1_temperature_counter == timer1_fifty_counter))
  {
    temperature_state = temperature_state_waited;
  }

  /* Count the silence */
  timer1_uart_idle_counter++;

  if (timer1_want_to_send_sms == 1 || 
      temperature_state == temperature_state_want_to_get ||
      temperature_state == temperature_state_waited)
  {
    /* I estimate that the 'safe-window' is about here */
    if (timer1_uart_idle_counter > 10 && timer1_uart_idle_counter < 40)
    {
      /* Do something */
      timer1_uart_idle_counter = 0;

      if (temperature_state == temperature_state_want_to_get)
      {
        temperature_request();

        /* Wait until the fifty counter gets back to its current value */
        timer1_temperature_counter = timer1_fifty_counter;
      }
      else if (temperature_state == temperature_state_waited)
      {
        temperature_retrieve();
      }

      if (timer1_want_to_send_sms == 1)
      {
        timer1_want_to_send_sms = 0;
        /* TODO: Add sms.c and function to take over the UART for sending */
      }
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
  /* TCCR1B: Prescaler to FCPU/256 & Enable  (Set bit CS12)  */
  TCCR1B |= _BV(CS12);

  /* Setup the Flashing LED : Put PD5 as an Output (pin5)    */
  DDRD |= _BV(DDD5);
}

