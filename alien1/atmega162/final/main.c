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

#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "main.h"
#include "camera.h"
#include "gps.h"
#include "log.h"
#include "radio.h"
#include "sms.h"
#include "statusled.h"
#include "timer1.h"
#include "timer3.h"
#include "watchdog.h"

int main()
{
  camera_init();
  gps_init();
  log_init();
  radio_init();
  sms_init();
  statusled_init();
  timer1_init();
  timer3_init();
  watchdog_init();

  /* Interrupts on - go go go! */
  sei();

  /* Now sleep - the whole program is interrupt driven */
  for (;;) sleep_mode();
}

