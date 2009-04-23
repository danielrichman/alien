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

#ifndef ALIEN_TEMPERATURE_HEADER
#define ALIEN_TEMPERATURE_HEADER

/* Global status variables & defines */
#define temperature_status_reset_pulse_l      0
#define temperature_status_reset_pulse_h      1
#define temperature_status_presence_pulse     2
#define temperature_status_skiprom_cmd        3
#define temperature_status_convtemp_cmd       4
#define temperature_status_convtemp           5
#define temperature_status_reset_pulse2_l     6
#define temperature_status_reset_pulse2_h     7
#define temperature_status_presence_pulse2    8
#define temperature_status_readscratch_cmd    9
#define temperature_status_readscratch        10
#define temperature_status_end                11

extern uint8_t temperature_status;

/* Prototypes */
void temperature_writeb(uint8_t b);
uint8_t temperature_read();
void temperature_get();
void temperature_init();

#endif 
