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

#ifndef __DEBUG_DEBUG_H__
#define __DEBUG_DEBUG_H__

#ifndef DEBUG
#define DEBUG 0
#endif

#if DEBUG

#include <stdint.h>
#include "buffer.h"

#define DEBUG_OK       BUFFER_OK
#define DEBUG_OVERFLOW BUFFER_OVERFLOW

void debug_init();
uint8_t debug_write(uint8_t *data, uint16_t len);

/* For static strings */
#define debug_es(str)  debug_write( (uint8_t *) str, sizeof(str))

#else

#define debug_init()
#define debug_write()
#define debug_es(str)

#endif

#endif
