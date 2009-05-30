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

/* The hexdump code is just macros. */
#ifndef ALIEN_HEXDUMP_HEADER
#define ALIEN_HEXDUMP_HEADER

/* In order to conserve space, I will try not to use printf at all on the 
 * flight computer for alien1. However, some sort of hexdump 
 * byte-to-2hexadecimal is needed. This is a test implementation */

/* Work out if it's 0-9 or A-F, then just add. If it's A-F we need to know 
 * how far it is past A; so we subtract 10. Then add to 'A'. Rearrange this
 * to ('A' - 10) + Byte, and GCC will simplify/pre-evaluate this to 55+byte */
#define num_to_char(number)   ((number) < 10 ?                           \
                                               ('0' + (number)) :        \
                                               (('A' - 10) + (number)) )

/* To select the 4 bits we do this */
#define first_four(byte)       (0x0F & (byte))

/* Last four: Shift left to get to a number < 16 */
#define  last_four(byte)      ((0xF0 & (byte)) >> 4)

/* The MSB goes first. */
#define hexdump_a(byte)  num_to_char( last_four(byte))
#define hexdump_b(byte)  num_to_char(first_four(byte))

/* Casts anything like a struct to something we can hexdump/treat
 * as byte-array */
#define ba(source)  ( (uint8_t *) (&(source)) )

#endif
