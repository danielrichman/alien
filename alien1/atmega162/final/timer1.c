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
#include <avr/wdt.h>
#include <stdint.h>
#include "timer1.h"
#include "camera.h"
#include "gps.h"
#include "messages.h"
#include "radio.h"
#include "sms.h"
#include "statusled.h"
#include "temperature.h"
#include "watchdog.h"

/* TIMER1 is used for many things. It is set up to generate a 50hz interrupt,
 * and each time it does interrupt radio_proc gets a call. Furthermore, every 
 * fifty interrupts ( = one second) the messages.c system gets a call, telling
 * it to distribute a message. Finally, every 60 seconds temperature.c gets
 * called, telling it to start reading temperature. Also, SMSes get distributed
 * by the logic inside the 50hz routine */

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

/* 50hz timer interrupt */
ISR (TIMER1_COMPA_vect)
{
  /* Temporary Variable, Probably will be optimised out */
  uint8_t i;

  /* At 50hz we want to trigger the radio. */
  radio_proc();

  /* Increment the counter */
  timer1_fifty_counter++;

  /* Reset the watchdog */
  wdt_reset();

  if (timer1_fifty_counter == 50)
  {
    /* One second has passed */
    timer1_fifty_counter = 0;

    /* Somethings to do each second: */
    camera_proc();                             /* Take pictures */
    statusled_proc();                          /* Flashy flashy */
    messages_push();                           /* Push Messages */
    latest_data.system_fix_age++;              /* Increment Age */

    /* set by gps.c, see messages.h */
    i = messages_get_gps_rx_ok();
    if (i != 0)
    {
      i--;
      messages_clear_gps_rx_ok();
      messages_set_gps_rx_ok(i);
    }

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

        sms_mode = sms_mode_rts;
      }
    }
  }

  /* Count the silence */
  timer1_uart_idle_counter++;

  /* Use these macros to try and make the logic a bit more readable */
  #define sms_idle     (sms_mode == sms_mode_null ||                          \
                        sms_mode == sms_mode_data ||                          \
                        sms_mode == sms_mode_rts)
  #define want_to_sms  (sms_mode == sms_mode_ready ||                         \
                        sms_mode == sms_mode_rts)
  #define temp_idle    (temperature_state == temperature_state_null ||        \
                        temperature_state == temperature_state_want_to_get)
  #define want_to_temp (temperature_state == temperature_state_want_to_get || \
                        temperature_state == temperature_state_waited)

  /* I estimate that the 'safe-window' is about here */
  if (timer1_uart_idle_counter > 15 && timer1_uart_idle_counter < 35)
  {
    if (want_to_sms && temp_idle)
    {
      /* Don't start sending smses while taking temperature! Both SMS and 
       * temperature use TIMER3! */
      sms_start();
    }

    if (want_to_temp && sms_idle)
    {
      /* Don't take temperature and while sending a sms! */
      if (temperature_state == temperature_state_want_to_get)
      {
        temperature_request();
      }
      else if (temperature_state == temperature_state_waited)
      {
        temperature_retrieve();
      }
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
  TCCR1B  = ((1 << WGM12) | (1 << CS12));
  TIMSK   =  (1 << OCIE1A);

  /* Temperature and SMS will be triggered at/every 1 minute and 5 minutes,
   * respectivly. We'd like to also do it at 0 minutes. */
  temperature_state = temperature_state_want_to_get;
  sms_mode = sms_mode_rts;
}

