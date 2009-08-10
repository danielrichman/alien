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
#include <avr/wdt.h>
#include "watchdog.h"
#include "messages.h"

void watchdog_init()
{
  /* Grab the source of the reset and log it: bits 3..0 in MCUCSR. Place in 
   * bits 7..4 in system_state */
  if (MCUCSR & (1 << WDRF))
  {
    messages_set_mcucsr_wdt();
  }

  /* Now reset the status register */
  MCUCSR = 0;

  /* Setup for a ~2.1 second watchdog reset */
  wdt_enable(WDTO_2S);
}

