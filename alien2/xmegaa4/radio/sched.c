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
#include <stdlib.h>

#include "../util.h"
#include "../data.h"

#include "radio.h"
#include "sched.h"

#include "domex.h"
#include "rtty.h"
#include "uplink.h"

/* Testing */
#include "hell.h"
#include "morse.h"
#include "../test.h"

struct radio_rotation_item
{
    const struct radio_state settings;
    uint8_t reps;
};

#define default_source test_source /* Testing */ /* telem_get_byte */
#define rotation_len 5 /* Testing */ /* 3 */
static struct radio_rotation_item rotation[rotation_len] =
    { { { &domex, default_source, 0 }, 2 },
      { { &rtty, default_source, 0 }, 1 },
/* Not yet implemented */
/*      { { &uplink, NULL, 0 }, 1 }, */
/* Testing: */
      { { &hell, default_source, 0 }, 1 },
      { { &rtty, default_source, 1 }, 2 },
      { { &morse, default_source, 0 }, 1 } };

#define TYPE_NONE     0
#define TYPE_ROTATION 1
#define TYPE_QUEUE    2

static uint8_t rotation_item, rotation_reps, current_type;
static struct radio_queue_item *queue_item, *queue_last_item;

void radio_queue_add(struct radio_queue_item *item)
{
    if (item->next != NULL)
    {
        return;
    }

    if (queue_item == NULL)
    {
        queue_item = item;
    }
    else
    {
        queue_last_item->next = item;
    }

    queue_last_item = item;
}

const struct radio_state *radio_sched_get()
{
    /* Dispose of the current item / update the current item */
    if (current_type == TYPE_ROTATION)
    {
        rotation_reps++;

        if (rotation_reps >= rotation[rotation_item].reps)
        {
            rotation_reps = 0;
            rotation_item++;

            if (rotation_item >= rotation_len)
            {
                rotation_item = 0;
            }
        }
    }
    else if (current_type == TYPE_QUEUE)
    {
        struct radio_queue_item *old_item;
        old_item = queue_item;
        queue_item = queue_item->next;
        old_item->next = NULL; /* signals completion */
    }

    /* Return the new item */
    if (queue_item != NULL)
    {
        current_type = TYPE_QUEUE;
        return &(queue_item->settings);
    }
    else
    {
        current_type = TYPE_ROTATION;
        return &(rotation[rotation_item].settings);
    }
}
