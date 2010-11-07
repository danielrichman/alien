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

#ifndef __RADIO_SCHED_H__
#define __RADIO_SCHED_H__

#include "radio.h"

/*
 * We force anything that uses queue_add to statically allocate its own memory
 * for the item so that we don't have to use dynamic memory
 */
struct radio_queue_item
{
    struct radio_state settings;
    struct radio_queue_item *next;
};

void radio_queue_add(struct radio_queue_item *item);
const struct radio_state *radio_sched_get();

#endif
