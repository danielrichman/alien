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

/* DEBUG: XXX */
#include "../debug/debug.h"

static void uplink_init();
static uint8_t uplink_interrupt();
static PGM_P uplink_getname(uint8_t t, uint8_t options);

const struct radio_mode uplink = { uplink_init, uplink_interrupt,
                                   uplink_getname };

/*
#define UPLINK_NOISECHK   0
#define UPLINK_CALLOPEN   1
#define UPLINK_REPORT     2
#define UPLINK_NOISECHKB  3
#define UPLINK_MAIN       4
#define UPLINK_CONFIRM    5
static uint8_t uplink_status;
*/

/* TESTING XXX */
static uint16_t uplink_samples;

static void uplink_init()
{
    /* 
     * Uplink is 50baud. We'll make 32 samples (but only 16 will count).
     * i.e., 1600baud:
     */
    radio_hw_mode(RADIO_HW_MODE_RX);
    radio_hw_timer_set(RADIO_HW_TIMER_DIV1, 5000);

    uplink_samples = 0;
}

static uint8_t uplink_interrupt()
{
    struct
    {
        uint8_t hdr;
        uint16_t af;
        uint16_t rssi;
        uint8_t tail;
    } message;

    message.hdr = 0xFC;
    message.tail = 0xF2;
    radio_hw_adc_get(&message.af, &message.rssi);
    debug_write((uint8_t *) &message, sizeof(message));

    uplink_samples++;

    if (uplink_samples == 51200)
        return RADIO_INTERRUPT_FINISHED;
    else
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
