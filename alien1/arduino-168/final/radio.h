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

#define radio_state_not_txing   0xFF
#define radio_state_start_bit   0xDD
#define radio_state_stop_bit    0xEE
#define radio_no_of_bits          7      /* 7bit ASCII */
/* 0x00 through 0x07 for radio_state represents bits */
/* Transmitting the stop bit sets the idle state too */

extern uint8_t radio_state;

/* Prototypes */
void radio_init();
void radio_send();
void radio_proc();

#endif

