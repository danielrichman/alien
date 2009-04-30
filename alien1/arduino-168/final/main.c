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
#include "temperature.h"  
#include "timer1.h"

int main()
{
  /* Interrupts off until we're ready to roll */
  cli();

  /* Setup everything (except SMS which competes with gps for 
   * the UART, is only enabled when it is needed */
  camera_init();
  gps_init();
  radio_init();
  timer1_init();
  temperature_init();

  /* Interrupts on - go go go! */
  sei();

  /* Now sleep - the whole program is interrupt driven */
  for (;;)    sleep_mode();
}

