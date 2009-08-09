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
#include <stdint.h>
#include "timer3.h"
#include "sms.h"
#include "temperature.h"

/* 1hz interrupt - enabled when needed */
ISR (TIMER3_COMPA_vect)
{
  /* Only tick once */
  timer3_stop();

  /* Update whatever we need to update */
  if (temperature_state == temperature_state_requested)
  {
    temperature_state = temperature_state_waited;
  }

  if (sms_mode == sms_mode_waiting)
  {
    sms_mode = sms_mode_ready;
  }
}

/* Note: we don't enable timer3 straight away. */
void timer3_init()
{
  /* For info, see timer1.c _init notes */
  OCR3A   = 62500;  /*  1Hz */
  ETIMSK  = (1 << OCIE3A);
}

