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

#ifndef __RADIO_RADIO_H__
#define __RADIO_RADIO_H__

#include <stdint.h>
#include <avr/pgmspace.h>
#include "../data.h"

#define RADIO_INTERRUPT_OK       0
#define RADIO_INTERRUPT_FINISHED 1
#define RADIO_INTERRUPT_DELAY    2

#define RADIO_NAME_SHORT 0
#define RADIO_NAME_LONG  1

/* RADIO_INTERRUPT_FINISHED must equal DATA_SOURCE_FINISHED */

typedef void (*radio_initialise_function)();
typedef uint8_t (*radio_interrupt_function)();
typedef PGM_P (*radio_getname_function)(uint8_t t, uint8_t options);

/*
 * There are "generic continuous data modes" and "one-shot modes";
 * SSTV is, for example, an "one-shot" mode: it starts, sends a picture,
 * then stops. Other data modes are capable of sending an arbitrary stream
 * of bytes. There may be special things inside this stream, e.g. SSDV, but
 * that's handled by genericdata.c
 */

struct radio_mode
{
    radio_initialise_function init;
    radio_interrupt_function isr;
    radio_getname_function getname;
};

struct radio_state
{
    const struct radio_mode *mode;
    data_source source;
    uint8_t options;
};

extern const struct radio_state *radio_current_state;
#define radio_current_source (radio_current_state->source)
#define radio_current_options (radio_current_state->options)

/*
 * For arbitrary use by the current running mode - to avoid every single
 * mode allocating one byte
 */
extern uint8_t radio_data_current_byte;
#define radio_data_update() radio_current_source(&radio_data_current_byte)

void radio_init();
void radio_isr();

#endif
