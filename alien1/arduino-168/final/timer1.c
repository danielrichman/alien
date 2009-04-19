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
 * called, telling it to start reading temperature, and 5 seconds after the 
 * reading is taken. */
/* Finally an LED on Arduino GPIO6 is flashed ;) */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdint.h>

#include "temperature.h"
#include "messages.h"
#include "hexdump.h"
#include "gps.h"
#include "radio.h"
#include "timer1.h"

#define FLASH_LED  PORTD ^= _BV(PD5)

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
    FLASH_LED;

    /* Increment the other counter */
    timer1_second_counter++;

    if (timer1_second_counter == 55)
    {
      /* Five seconds before the deadline... */
      temperature_request_reading();
    }

    if (timer1_second_counter == 60)
    {
      /* Reached the end of the minute */
      timer1_second_counter = 0;

      /* Something to do every minute */
      temperature_retrieve_reading();
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

