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
extern uint8_t temperature_state;

#define temperature_state_null            0
#define temperature_state_want_to_get     1
#define temperature_state_requested       2
#define temperature_state_waited          3

/* Bits in the MSB of the temperature to signal things (they arn't used) */
#define temperature_ubits_age             0x80
#define temperature_ubits_err             0x40

/* Prototypes */
void temperature_request();
void temperature_retrieve();
void temperature_reset();
void temperature_writebyte(uint8_t db);
void temperature_readbyte(uint8_t *ext_target, uint8_t *int_target);
uint8_t temperature_readbit();
void temperature_crcpush(uint8_t bit, uint8_t *crc);
void temperature_init();

#endif 
