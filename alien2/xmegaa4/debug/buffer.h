/*
    Copyright (C) 2011  Daniel Richman

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

#ifndef __DEBUG_BUFFER_H__
#define __DEBUG_BUFFER_H__

#include <stdint.h>
#include "debug.h"

#if DEBUG

#define BUFFER_OK       0
#define BUFFER_OVERFLOW 1

uint8_t buffer_write(uint8_t *data, uint16_t len);
uint8_t buffer_read_byte(uint8_t *b);

#endif

#endif
