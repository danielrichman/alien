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
#include "uplink.h"

static void uplink_init();
static uint8_t uplink_interrupt();
static PGM_P uplink_getname(uint8_t t, uint8_t options);

const struct radio_mode uplink = { uplink_init, uplink_interrupt,
                                   uplink_getname };

static void uplink_init()
{
    radio_hw_mode(RADIO_HW_MODE_RX);
}

static uint8_t uplink_interrupt()
{
    /* TODO */
    return RADIO_INTERRUPT_OK;
}

static char uplink_short_name[] PROGMEM = "UPL";
static char uplink_long_name[] PROGMEM = "Uplink";

static PGM_P uplink_getname(uint8_t t, uint8_t options)
{
    if (t == RADIO_NAME_SHORT)
    {
        return uplink_short_name;
    }
    else
    {
        return uplink_long_name;
    }
}
