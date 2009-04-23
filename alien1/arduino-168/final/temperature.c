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
#define TEMP_EXT_RELEASE  DDRD  &= ~(_BV(DDD6))   /* Set to input  */
#define TEMP_EXT_PULLLOW  DDRD  |=   _BV(DDD6)    /* Set to output */
#define TEMP_EXT_READ     PIND  &    _BV(PIND6)   /* Read bit 6    */

#define TEMP_INT_RELEASE  DDRD  &= ~(_BV(DDD7))
#define TEMP_INT_PULLLOW  DDRD  |=   _BV(DDD7)
#define TEMP_INT_READ     PIND  &    _BV(PIND7)

/* TODO: Write to latest_data.system_temp; don't forget .temperature_age */

#define skiprom_cmd        0xCC
#define convtemp_cmd       0x44
#define readscratch_cmd    0xBE

#define temperature_flags_ext_bad  0x01
#define temperature_flags_int_bad  0x02

#define temperature_ext_read       0x01
#define temperature_int_read       0x02

uint8_t temperature_status, temperature_substatus;
uint8_t temperature_crc, temperature_flags, temperature_byte;

uint16_t temperature_external, temperature_internal;

ISR (TIMER0_COMPA_vect)
{
  switch(temperature_status)
  {
    case temperature_status_reset_pulse_l:
    case temperature_status_reset_pulse2_l:
      TEMP_EXT_PULLLOW;
      TEMP_INT_PULLLOW;

      temperature_status++;

      OCR0A = 140;     /* 560us wait */
      break;

    case temperature_status_reset_pulse_h:
    case temperature_status_reset_pulse2_h:
      TEMP_EXT_RELEASE;
      TEMP_INT_RELEASE;

      temperature_status++;

      OCR0A = 25;      /* 100us wait */
      break;

    case temperature_status_presence_pulse:
    case temperature_status_presence_pulse2:
      if (TEMP_INT_READ)
      {
        /* If it's high, then the sensor isn't there - fail! */
        temperature_flags |= temperature_flags_int_bad;
      }
      if (TEMP_EXT_READ)
      {
        temperature_flags |= temperature_flags_ext_bad;
      }

      temperature_status++;

      OCR0A = 60;     /* 240us wait (we've already gone 40us into the
                       * presence pulse with the 100us wait above */
      break;

    case temperature_status_skiprom_cmd:
      temperature_writeb(skiprom_cmd);
      break;

    case temperature_status_convtemp_cmd:
      temperature_writeb(convtemp_cmd);
      break;

    case temperature_status_convtemp:

      /* TODO: Wait ages. Drop back to timer1.c - can't wait here */

      break;

    case temperature_status_readscratch_cmd:
      temperature_writeb(readscratch_cmd);
      break;

    case temperature_status_readscratch:
      /* TODO temperature_status_readscratch */
      break;

    case temperature_status_end:
      /* TODO temperature_status_end */
      break;
  }
}

/* Because the timing must be accurate for RW we don't leave the interrupt */

void temperature_writeb(uint8_t b)
{
  /* Select the required byte */
  b &= _BV(temperature_substatus);

  /* If we are writing low then pull it down for the full 64us, 
   * release and wait a tiny bit for it to be surely high.
   * If we are writing high then pull it down for 4us then release,
   * and wait out the rest of the 72us (72 - 4 = 68) */

  TEMP_EXT_PULLLOW;
  TEMP_INT_PULLLOW;

  if (b)  _delay_us(4);            /* Wait 4us    */
  else    _delay_us(64);           /* Wait 64us   */

  TEMP_EXT_RELEASE;
  TEMP_INT_RELEASE;

  if (b)  _delay_us(68);           /* Wait out the rest of the slot */
  else    _delay_us(8);            /* Wait 8us for it to come high  */

  temperature_substatus++;

  if (temperature_substatus == 8)
  {
    temperature_status++;
    temperature_substatus = 0;
  }
}

uint8_t temperature_read()
{
  uint8_t d;

  d = 0;   /* Initialise */

  TEMP_EXT_PULLLOW;
  TEMP_INT_PULLLOW;

  _delay_us(1.5);                          /* Wait 1.5us */

  TEMP_EXT_RELEASE;
  TEMP_INT_RELEASE;

  _delay_us(12);                           /* Wait 12us  */

  /* Read */
  if (TEMP_EXT_READ)     d |= temperature_ext_read;
  if (TEMP_INT_READ)     d |= temperature_int_read;

  /* Wait for the slot to end: Wait another 52us */
  _delay_us(52);

  return d;
}

void temperature_get()
{
  temperature_status = temperature_status_reset_pulse_h;
  temperature_flags  = 0;   /* Reset the flags */

  /* Perform the first step of the reset pulse and start timer */
  TEMP_EXT_PULLLOW;     /* Grab the 1wire bus and pull it low */
  TEMP_INT_PULLLOW;

  TCNT0   = 0;                          /* Reset counter */
  OCR0A   = 140;                        /* Interrupt in 560 microseconds */

  TCCR0B  = ((_BV(CS00)) | _BV(CS01));  /* Enable timer at FCPU/64 */

  /* The timer is now ticking at 250000hz, so all our microsecond timings
   * get divided by 4. (ie for 60microseceonds set OCR0A = 60/4 = 15) */
}

/* We only have to do this once. Setting it low and leaving it means that 
 * when we put outputmode (PULLLOW) then DQ gets grounded; When we set
 * input mode (RELEASE) no extra internal pullups are turned on and
 * DQ floats high */
#define TEMP_EXT_LOW      PORTD &= ~(_BV(PD6))
#define TEMP_INT_LOW      PORTD &= ~(_BV(PD7))

void temperature_init()
{
  /* Set timer0 settings. Timer0 gets reset and enabled in temperature_get() */
  TIMSK0 |= _BV(OCIE0A);  /* Enable compare match interrupts */
  TCCR0A |= _BV(WGM01);   /* Clear timer on compare match */

  /* Initialise the 1wire as released, don't turn on internal pullups.
   * The external 4k7 will pull it high. */
  TEMP_EXT_RELEASE;
  TEMP_EXT_RELEASE;
  TEMP_EXT_LOW;
  TEMP_INT_LOW;
}

