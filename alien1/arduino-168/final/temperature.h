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
#define temperature_step_reset_pulse        0
#define temperature_step_presence_pulse     1
#define temperature_step_skiprom_cmd        2
#define temperature_step_convtemp_cmd       3
#define temperature_step_convtemp           4
#define temperature_step_reset_pulse2       5
#define temperature_step_presence_pulse2    6
#define temperature_step_readscratch_cmd    7
#define temperature_step_readscratch        8
#define temperature_step_end                9

#define temperature_status_something        0   /* TODO */

extern uint8_t temperature_step;

/* Prototypes */
void temperature_get();
void temperature_init();

#endif 
