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
#include <stdint.h>
#include <stdlib.h>

#include "gps.h"  
#include "hexdump.h"
#include "messages.h"  
#include "radio.h" 
#include "sms.h"
#include "temperature.h"  
#include "timer1.h"

/* We will have temperature sensors on GPIO6 and GPIO7 (PD6 and PD7) - 
 * While I appreciate that you can have more sensors on one 1wire, we're
 * not exactly short for GPIOs and this means that we don't have to mess
 * around with ROM and SELECT commands to talk to each sensor individually */
/* External Temperature will be PD6, Internal Temperature will be PD7 */

/* Note: Don't forget to set to LOW before putting as an input */
#define TEMP_EXT_LOW      PORTD &= ~(_BV(PD6))    /* Pull DQ Low   */
#define TEMP_EXT_HIGH     PORTD |=   _BV(PD6)     /* Pull DQ High  */
#define TEMP_EXT_RELEASE  DDRD  &= ~(_BV(DDD6))   /* Set to input  */
#define TEMP_EXT_GRAB     DDRD  |=   _BV(DDD6)    /* Set to output */

#define TEMP_INT_LOW      PORTD &= ~(_BV(PD7))
#define TEMP_INT_HIGH     PORTD |=   _BV(PD7)
#define TEMP_INT_RELEASE  DDRD  &= ~(_BV(DDD7))
#define TEMP_INT_GRAB     DDRD  |=   _BV(DDD7)

/* TODO: Write to latest_data.system_temp; don't forget .temperature_age */

#define skiprom_cmd        0xCC
#define convtemp_cmd       0x44
#define readscratch_cmd    0xBE

uint8_t temperature_step, temperature_status, temperature_substatus;
uint8_t temperature_crc;

uint16_t temperature_external, temperature_internal;

void temperature_get()
{
  temperature_step   = temperature_step_reset_pulse;
  temperature_status = 1;   /* We do the first step in this function */

  /* Perform the first step of the reset pulse and start timer */
  TEMP_EXT_GRAB;      /* Grab the 1wire bus and pull it low */
  TEMP_EXT_LOW;
  TEMP_INT_GRAB;
  TEMP_INT_LOW;

  TCNT0   = 0;                          /* Reset counter */
  OCR0A   = 140;                        /* Interrupt in 560 microseconds */

  TCCR0A  = ((_BV(CS01)) | _BV(CS00));  /* Enable timer at FCPU/64 */

  /* The timer is now ticking at 250000hz, so all our microsecond timings
   * get divided by 4. (ie for 60microseceonds set OCR0A = 60/4 = 15) */
}

void temperature_init()
{
  /* Set timer0 settings. Timer0 gets reset and enabled in temperature_get() */
  TIMSK0 |= _BV(OCIE0A);  /* Enable compare match interrupts */
  TCCR0A |= _BV(WGM02);   /* Clear timer on compare match */
}

