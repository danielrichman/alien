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

#include <stdint.h>
#include <avr/pgmspace.h>
#include "data.h"

static uint8_t test_string[] PROGMEM =
    "Hello, world! This is the ALIEN-2 test program. It will output this "
    "string in a variety of modes: DominoEX22, RTTY50, RTTY300, "
    "Feldhellschreiber and Morse.\n"
    "I plan to also implement an uplink in rtty and SSTV\n";
static uint8_t test_position;

uint8_t test_source(uint8_t *b)
{
    uint8_t data;
    data = pgm_read_byte(&(test_string[test_position]));

    if (data == '\0')
    {
        test_position = 0;
        return DATA_SOURCE_FINISHED;
    }
    else
    {
        *b = data;
        test_position++;
        return DATA_SOURCE_OK;
    }
}
