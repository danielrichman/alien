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

#ifndef ALIEN_RADIO_HEADER
#define ALIEN_RADIO_HEADER

#include <stdint.h>

#define radio_state_not_txing   0
#define radio_state_start_bit   1
#define radio_state_data_bits   2
#define radio_state_stop_bit_a  3
#define radio_state_stop_bit_b  4
#define radio_state_pause       5

/* Transmitting the stop bit sets the idle state too, a MARK. */

extern uint8_t radio_state;

/* Prototypes */
void radio_init();
void radio_send();
void radio_proc();

#endif

