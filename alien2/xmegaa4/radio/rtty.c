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
#include "rtty.h"

static void rtty_init();
static uint8_t rtty_interrupt();
static void rtty_pause();
static PGM_P rtty_getname(uint8_t t, uint8_t options);

const struct radio_mode rtty = { rtty_init, rtty_interrupt, rtty_getname };

#define WARM_UP    0
#define WARMED_UP  1
#define START_BIT  2
#define DATA_BITS  3
/* 8bit ascii data bits 3,4,5,6,7,8,9,10 */
#define STOP_BIT_A 11
#define STOP_BIT_B 12
#define STATUS_END 13
static uint8_t rtty_status;

#define SPACE_VALUE 2000
#define MARK_VALUE  2700

static void rtty_init()
{
    radio_hw_set_mode(RADIO_HW_MODE_TX);
    radio_hw_set_dac(MARK_VALUE);
    rtty_pause();

    radio_data_update();
    rtty_status = WARM_UP;
}

#define SPACE 0
#define MARK  1

static uint8_t rtty_interrupt()
{
    uint8_t bit;

    if (rtty_status == WARM_UP)
    {
        bit = MARK;
    }
    else if (rtty_status == WARMED_UP)
    {
        if (radio_current_options == RTTY_SLOW)
        {
            radio_hw_set_speed(RADIO_HW_TIMER_DIV4, 40000);
        }
        else
        {
            radio_hw_set_speed(RADIO_HW_TIMER_DIV1, 26667);
        }

        bit = MARK;
    }
    else if (rtty_status == START_BIT)
    {
        bit = SPACE;
    }
    else if (rtty_status == STOP_BIT_A || rtty_status == STOP_BIT_B)
    {
        bit = MARK;
    }
    else
    {
        uint8_t mask;
        mask = (1 << (rtty_status - (DATA_BITS)));

        if (radio_data_current_byte & mask)
        {
            bit = MARK;
        }
        else
        {
            bit = SPACE;
        }
    }

    if (bit == SPACE)
    {
        radio_hw_set_dac(SPACE_VALUE);
    }
    else
    {
        radio_hw_set_dac(MARK_VALUE);
    }

    rtty_status++;

    if (rtty_status >= STATUS_END)
    {
        uint8_t source_status;
        source_status = radio_data_update();

        if (source_status == DATA_SOURCE_FINISHED)
        {
            rtty_pause();
            return RADIO_INTERRUPT_FINISHED;
        }
        else
        {
            rtty_status = START_BIT;
        }
    }

    return RADIO_INTERRUPT_OK;
}

static void rtty_pause()
{
    radio_hw_set_speed(RADIO_HW_TIMER_DIV256, 15625);
}

static char rtty_slow_short_name[] PROGMEM = "RTTY50";
static char rtty_slow_long_name[] PROGMEM = "RTTY 50 425 8n2";
static char rtty_fast_short_name[] PROGMEM = "RTTY300";
static char rtty_fast_long_name[] PROGMEM = "RTTY 300 425 8n2";

static PGM_P rtty_getname(uint8_t t, uint8_t options)
{
    if (t == RADIO_NAME_SHORT)
    {
        if (options == RTTY_SLOW)
        {
            return rtty_slow_short_name;
        }
        else
        {
            return rtty_fast_short_name;
        }
    }
    else
    {
        if (options == RTTY_SLOW)
        {
            return rtty_slow_long_name;
        }
        else
        {
            return rtty_fast_long_name;
        }
    }
}
