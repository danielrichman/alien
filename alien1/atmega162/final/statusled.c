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

#include "camera.h"
#include "gps.h"
#include "hexdump.h"
#include "log.h"
#include "main.h"
#include "messages.h"
#include "radio.h"
#include "sms.h"
#include "statusled.h"
#include "temperature.h"
#include "timer1.h"
#include "timer3.h"
#include "watchdog.h"

/* There's a tri-colour LED on PA0 and PA1.
 * PA0 drives a red led, PA1 drives a green one. By using the 4 possible
 * combinations of PA0/1, we can have off, red, green and yellow lights.
 * statusled.c will flick between two states. */
/* The macros that generate and decode these combinations are in the header */

uint8_t statusled_flash;

#define statusled_flash_a     0
#define statusled_flash_b     1

/* LED controlling macros */
#define STATUSLED_RED_ON   PORTA |=   _BV(PA0)
#define STATUSLED_RED_OFF  PORTA &= ~(_BV(PA0))
#define STATUSLED_GRN_ON   PORTA |=   _BV(PA1)
#define STATUSLED_GRN_OFF  PORTA &= ~(_BV(PA1))

void statusled_proc()
{
  /* We can tell if there has been a write to the latest_data.system_location 
   * because at poweron it will have been set to \0. */
  if (latest_data.system_location.time[0] != '\0')
  {
    if (statusled_flash == statusled_flash_a)
    {
      /* A green flash starts both of these statuses */
      STATUSLED_RED_OFF;
      STATUSLED_GRN_ON;

      statusled_flash = statusled_flash_b;
    }
    else
    {
      if (latest_data.system_location.fix_age < 2)
      {
        if ( (latest_data.system_temp.external_temperature & 
                                       temperature_ubits_valid) &&
             (latest_data.system_temp.internal_temperature & 
                                       temperature_ubits_valid) &&
            !(latest_data.system_temp.external_temperature & 
                                       temperature_ubits_err) &&
            !(latest_data.system_temp.internal_temperature & 
                                       temperature_ubits_err))
        {
          /* Green/Off pulsing to show that the gps and temp are good */
          STATUSLED_RED_OFF;
          STATUSLED_GRN_OFF;
        }
        else
        {
          /* Green/Yellow pulsing to show that gps is good but temp isn't */
          STATUSLED_RED_ON;
          STATUSLED_GRN_ON;
        }
      }
      else
      {
        /* Green/Red pulsing to show that we had a fix but it's lost */
        STATUSLED_RED_ON;
        STATUSLED_GRN_ON;
      }

      statusled_flash = statusled_flash_a;
    }
  }
  else
  {
    /* Otherwise we pulse red/off or yellow/off depending on 
     * if it was good reset (yellow = Watchdog or Brown-out reset) */
    STATUSLED_GRN_OFF;

    if (statusled_flash == statusled_flash_a)
    {
      if (log_header & ((_BV(WDRF)) | (_BV(BORF))))
      {
        STATUSLED_GRN_ON;
      }

      STATUSLED_RED_ON;
      statusled_flash = statusled_flash_b;
    }
    else
    {
      STATUSLED_RED_OFF;
      statusled_flash = statusled_flash_a;
    }
  }
}

void statusled_init()
{
  /* Set both as outputs */
  DDRA |= ((_BV(PA0)) | (_BV(PA1)));
}

