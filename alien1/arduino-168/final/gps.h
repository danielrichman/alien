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

#ifndef ALIEN1_GPS_HEADER
#define ALIEN1_GPS_HEADER

#include <stdint.h>
#include "messages.h"

/* Prototypes */
void gps_proc_byte(uint8_t c);
void gps_next_field();
void gps_init();

/* GPS data struct is defined in messages.h */

/* Other things need access to this in order to take the data away, 
 * specifically messages.c and messages_gps_data_push()             */
extern gps_information gps_data;

#endif 

