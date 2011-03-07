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
#include <stdlib.h>
#include <avr/pgmspace.h>

#include "../util.h"
#include "../data.h"

#include "radio.h"
#include "hardware.h"
#include "sched.h"

#include "morse.h"

#define STATUS_INIT         0  /* morse announce */
#define STATUS_PREDELAY     1
#define STATUS_RUNNING      2
#define STATUS_FINISHED     3  /* data announce, or -> running or post delay */
#define STATUS_POSTDELAY    4
#define STATUS_END          5

static void item_finished();
static void initialise_wait();
static void announce_source_init(uint8_t t);
static uint8_t announce_source(uint8_t *b);

const struct radio_state announce_morse = { &morse, announce_source, 0 };
struct radio_state announce_data = { NULL, announce_source, 0 };

const struct radio_state *radio_current_state;
static const struct radio_state *next_state;
static uint8_t radio_status;
static uint8_t current_item_status = RADIO_INTERRUPT_OK;

uint8_t radio_data_current_byte;

void radio_init()
{
    radio_hw_init();

    /* item_finished will wrap around to INIT */
    radio_status = STATUS_END;
    next_state = radio_sched_get();
    item_finished();
}

void radio_isr()
{
    /*
     * items return FINISHED just after they've done their final action,
     * so we have to wait until the isr triggers again before we initialise
     * the next item.
     *
     * get a new item if the current one has FINISHED, or if we're currently
     * DELAYing
     */
    if (unlikely(current_item_status != RADIO_INTERRUPT_OK))
    {
        item_finished();

        if (current_item_status == RADIO_INTERRUPT_DELAY)
        {
            /* Don't try to dereference radio_current_item, it's NULL */
            return;
        }
    }

    current_item_status = radio_current_state->mode->isr();
}

static void item_finished()
{
    radio_status++;

    if (radio_status >= STATUS_END)
    {
        radio_status = STATUS_INIT;
    }

    switch (radio_status)
    {
        case STATUS_INIT:
            announce_source_init(RADIO_NAME_SHORT);
            radio_current_state = &announce_morse;
            break;

        case STATUS_PREDELAY:
        case STATUS_POSTDELAY:
        postdelay_jump:
            initialise_wait();
            break;

        case STATUS_RUNNING:
        running_jump:
            radio_current_state = next_state;
            break;

        case STATUS_FINISHED:
            next_state = radio_sched_get();

            /*
             * If the next item's mode is the same as this one, jump back to
             * RUNNING. Otherwise, leave status as FINISHED and select 
             * announce_data. If this mode cannot announce_data, jump past
             * it to POSTDELAY.
             */
            if (next_state->mode == radio_current_state->mode &&
                next_state->options == radio_current_state->options)
            {
                radio_status = STATUS_RUNNING;
                goto running_jump;
            }
            else if (0 /* TODO can't announce data */)
            {
                radio_status = STATUS_POSTDELAY;
                goto postdelay_jump;
            }
            else
            {
                announce_source_init(RADIO_NAME_LONG);
                announce_data.mode = radio_current_state->mode;
                announce_data.options = radio_current_state->options;
                radio_current_state = &announce_data;
            }
            break;
    }

    if (radio_current_state != NULL)
    {
        current_item_status = RADIO_INTERRUPT_OK;
        radio_current_state->mode->init();
    }
}

static void initialise_wait()
{
    current_item_status = RADIO_INTERRUPT_DELAY;
    radio_current_state = NULL;
    radio_hw_timer_set(RADIO_HW_TIMER_DIV256, 31250);
    radio_hw_mode(RADIO_HW_MODE_IDLE);
}

#define ASTATUS_HEADER 0
#define ASTATUS_NAME   1

static char announce_header[] PROGMEM = "--- Switching to mode ---";

static PGM_P announce_name;
static uint8_t announce_pos, announce_status;

static void announce_source_init(uint8_t t)
{
    if (t == RADIO_NAME_SHORT)
    {
        /* Skip header */
        announce_status = ASTATUS_NAME;
    }
    else
    {
        announce_status = ASTATUS_HEADER;
    }

    announce_pos = 0;
    announce_name = next_state->mode->getname(t, next_state->options);
}

static uint8_t announce_source(uint8_t *b)
{
    PGM_P s;
    uint8_t c;

    if (announce_status == ASTATUS_HEADER)
    {
        s = announce_header;
    }
    else
    {
        s = announce_name;
    }

    c = pgm_read_byte(&(s[announce_pos]));
    announce_pos++;

    if (c == '\0')
    {
        if (announce_status == ASTATUS_HEADER)
        {
            announce_pos = 0;
            announce_status = ASTATUS_NAME;
            c = '\n';
        }
        else
        {
            return DATA_SOURCE_FINISHED;
        }
    }

    *b = c;
    return DATA_SOURCE_OK;
}
