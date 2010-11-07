/*
    Copyright (C) 2010  Daniel Richman

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

#include <stdint.h>
#include <avr/pgmspace.h>

#include "radio.h"
#include "hardware.h"
#include "sstv.h"

static void sstv_init();
static uint8_t sstv_interrupt();
static PGM_P sstv_getname(uint8_t t, uint8_t options);

const struct radio_mode sstv = { sstv_init, sstv_interrupt, sstv_getname };

static void sstv_init()
{
    /* TODO */
}

static uint8_t sstv_interrupt()
{
    /* TODO */
    return RADIO_INTERRUPT_FINISHED;
}

static char sstv_short_name[] PROGMEM = "SSTV";
static char sstv_long_name[] PROGMEM = "SSTV Martin1";

static PGM_P sstv_getname(uint8_t t, uint8_t options)
{
    if (t == RADIO_NAME_SHORT)
    {
        return sstv_short_name;
    }
    else
    {
        return sstv_long_name;
    }
}
