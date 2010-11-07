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

#ifndef __DATA_H__
#define __DATA_H__

#include <stdint.h>

#define DATA_SOURCE_OK       0
#define DATA_SOURCE_FINISHED 1

/*
 * writes a new byte to b and returns 0, or 
 * returns 1 to indicate the end of the stream
 */
typedef uint8_t (*data_source)(uint8_t *b);

#endif
