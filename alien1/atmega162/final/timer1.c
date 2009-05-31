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
#include "log.h"
#include "main.h"
#include "messages.h"  
#include "radio.h" 
#include "sms.h"
#include "temperature.h"  
#include "timer1.h"
#include "timer3.h"

/* Although the gps and the sms do not compete for a UART, we do this to 
 * try and make sure each module has as much time as possible to execute
 * ie. they don't compete for processor time. Therefore, we use this to 
 * work out when it's the best window of opportunity to send a SMS.
 * gps.c zeros the idle counter it whenever it recieves a char, so if it 
 * reaches a high value we know that the GPS UART is idle and we can
 * start processing other things */
uint8_t timer1_uart_idle_counter;

/* These divide the 50hz into seconds, minutes and 5-minutes */
uint8_t timer1_fifty_counter, timer1_second_counter, timer1_minute_counter;

/* TODO: Perhaps some sort of watch dog?
 * Setup the hardware WDT and reset it in our 50hz interrupt, so if one 
 * of the modules locks up we can grab it
 * Furthermore, perhaps every minute we could have a watchdog-check-module
 * that checks the age on the GPS, age on the temp etc. to see if it's 
 * failing to get a fix and powercycle it or something. 
 * Also, let's have an advanced status-led system - PA1 and PA0 will 
 * connect to a tri-colour led. */

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
    camera_proc();                             /* Take pictures */
    messages_push();                           /* Push Messages */
    latest_data.system_location.fix_age++;     /* Increment Age */

    /* Increment the other counter */
    timer1_second_counter++;

    if (timer1_second_counter == 60)
    {
      /* Reached the end of the minute */
      timer1_second_counter = 0;

      /* Something to do (roughly) every minute */
      temperature_state = temperature_state_want_to_get;

      /* Increment the minute counter */
      timer1_minute_counter++;

      if (sms_mode == sms_mode_null && timer1_minute_counter == 5)
      {
        /* Every five minutes ... */
        timer1_minute_counter   = 0;

        sms_data = latest_data;
        sms_mode = sms_mode_rts;
      }
    }
  }

  /* Count the silence */
  timer1_uart_idle_counter++;

  /* I estimate that the 'safe-window' is about here */
  if (timer1_uart_idle_counter > 15 && timer1_uart_idle_counter < 35)
  {
    if (sms_mode == sms_mode_null || sms_mode == sms_mode_rts)
    {
      /* Don't take temperature and while sending a sms! t3 is required */
      if (temperature_state == temperature_state_want_to_get)
      {
        temperature_request();
      }
      else if (temperature_state == temperature_state_waited)
      {
        temperature_retrieve();
      }
    }

    if (temperature_state == temperature_state_null && 
        (sms_mode == sms_mode_ready || sms_mode == sms_mode_rts))
    {
      /* Don't start sending smses while taking temperature! */
      sms_start();
    }
  }
}

void timer1_init()
{
  /* Prescaler will be FCPU/256 (Set bit CS02). 
   * So Timer freq will be 16000000/256 = 62500Hz */
  OCR1A   = 1250;   /* 50Hz: 62500/50 = 1250 */

  /* (E)TIMSK:  Enable Compare Match Interrupts (Set bit OCIEnA) *
   * TCCRnB:    Clear timer on compare match    (Set bit WGMn2)  *
   * TCCRnB:    Prescaler to FCPU/256 & Enable  (Set bit CSn2)   */
  TCCR1B  = _BV(WGM12)  | _BV(CS12);
  TIMSK   = _BV(OCIE1A);
}

