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

#include <stdint.h>
#include <string.h>

#include "../data.h"
#include "debug.h"
#include "buffer.h"

#if DEBUG

static uint8_t buffer[512];
static uint16_t buffer_pos;
static uint16_t buffer_fill;

uint8_t buffer_write(uint8_t *data, uint16_t len)
{
    uint16_t write_from;

    if (buffer_fill + len > sizeof(buffer) - 1)
    {
        return BUFFER_OVERFLOW;
    }

    write_from = buffer_pos + buffer_fill;

    if (write_from >= sizeof(buffer))
    {
        write_from -= sizeof(buffer);
    }

    if (write_from + len > sizeof(buffer))
    {
        uint16_t chunk_one = sizeof(buffer) - write_from;
        uint16_t chunk_two = len - chunk_one;

        memcpy(&buffer[write_from], &data[0], chunk_one);
        memcpy(&buffer[0], &data[chunk_one], chunk_two);
    }
    else
    {
        memcpy(&buffer[write_from], &data[0], len);
    }

    buffer_fill += len;
    return BUFFER_OK;
}

uint8_t buffer_read_byte(uint8_t *b)
{
    if (buffer_fill == 0)
    {
        return DATA_SOURCE_FINISHED;
    }

    *b = buffer[buffer_pos];
    buffer_pos++;
    buffer_fill--;

    if (buffer_pos >= sizeof(buffer))
    {
        buffer_pos -= sizeof(buffer);
    }

    return DATA_SOURCE_OK;
}

#endif
