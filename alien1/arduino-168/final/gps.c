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

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdint.h>

#include "messages.h"
#include "hexdump.h"
#include "gps.h"

/* A list of fields and their index, starting from 1. The index goes up
 * every time a ',' or a '.' is encountered, and it also goes up to separate
 * the degrees and minutes (manual code below) */
#define gps_state_null            0    /* Invalidated status, waiting for $ */
#define gps_state_sentence_name   1    /* Eg GPGGA */
#define gps_state_time            2
#define gps_state_lat_d           4
#define gps_state_lat_m           5
#define gps_state_lat_s           6
#define gps_state_lat_dir         7
#define gps_state_lon_d           8
#define gps_state_lon_m           9
#define gps_state_lon_s           10
#define gps_state_lon_dir         11
#define gps_state_satc            13
#define gps_state_alt             16
#define gps_state_alt_u           17
#define gps_state_checksum        21

/* substate is used for counting through each uint8_t */
uint8_t gps_state, gps_checksum, gps_substate, gps_storing_maxlen;
uint8_t *gps_storing_location;

/* GPGGA sentences provide fix data */
#define gps_sentence_name_length 5
uint8_t gps_sentence_mask[gps_sentence_name_length] = 
                                        { 'G', 'P', 'G', 'G', 'A' };

/* Working data location - while we're recieving it goes here. */
gps_information gps_data;

/* TODO: Put this on the correct ISR and have it read the UART register */
void gps_proc_byte(uint8_t c)
{
  uint8_t i, j;  /* General purpose temporary variables, used in the for loop
                    that clears gps_data and the checksum checks.
                    Should be optimised out. */

  /* We treat the $ as a reset pulse. This overrides the current state because
   * a) a $ isn't valid in any of our data fields
   * b) if a $ is sent by accident/corruptified then the next sentence 
   * might get ignored */
  if (c == '$')
  {
    /* Sentence beginning! gogogo! */
    gps_state = gps_state_null;
    gps_checksum = 0;
    gps_next_field();

    /* Reset the gps_data struct! Use c as a temp var */
    for (i = 0; i < sizeof(gps_data); i++)
      ((uint8_t *) &gps_data)[i] = 0;

    return;  /* Discard the $, wait for next char. */
  }

  if (gps_state == gps_state_null)
  {
    return;  /* Discard this char. */
  }
  else if (gps_state == gps_state_checksum)
  {
    /* J represents what C should be. */

    switch (gps_substate)
    {
      case 0:
        j = '*';
        break;
      case 1:
        j = last_four(gps_checksum);
        j = num_to_char(j);
        break;
      case 2:
        j = first_four(gps_checksum);
        j = num_to_char(j);
        break;
      default:
        j = 0;
        break;
    }

    if (c != j)
    {
      /* Its trashed or invalid. Boo Hoo; discard!! */
      gps_state = gps_state_null;
      return;
    }

    /* Although this is repetition, rather do it here than waste cpu
     * time trawling down to the bottom to do it... ;) */
    gps_substate++;

    if (gps_substate == 3)
    {
      /* GPS data updated, send it to the messages manager. */
      /* The data will be taken from gps_data */
      messages_gps_data_push();

      /* Reset, ready for the next sentence */
      gps_state = gps_state_null;
    }

    return;
  }
  else
  {
    gps_checksum ^= c;
  }

  if (c == '.' || c == ',')
  {
    /* If its a , a . then disregard current char and bump the status +1 */
    gps_next_field();
    return;
  }

  if ( (gps_state == gps_state_lat_d && gps_substate == 2) ||
       (gps_state == gps_state_lon_d && gps_substate == 3) )
  {
    /* If it's the degrees field of lat or long & it's the 2/3rd char, then we
     * need to move onto the minutes, but DON'T return & discard this char */
    gps_next_field();
  }

  /* For the fields that are a simple copy-paste... */
  if (gps_storing_maxlen != 0)
  {
    if (gps_substate == gps_storing_maxlen)
    {
      /* DON'T OVERFLOW! */
      gps_state = gps_state_null;
      return;
    }

    if (c < '0' || c > '9')
    {
      /* Invalid char */
      gps_state = gps_state_null;
      return;
    }

    gps_storing_location[gps_substate] = c;
  }
  else
  {
    /* For the fields that require manual processing... */
    switch (gps_state)
    {
      case gps_state_sentence_name:
        if (c != gps_sentence_mask[gps_substate])
        {
          /* Wrong type of sentence for us, thx */
          gps_state = gps_state_null;
          return;
        }

        break;

      case gps_state_lat_dir:
        if (c == 'N')
        {
          gps_data.flags |= gps_cflag_north;
        }
        else if (c == 'S')
        {
          gps_data.flags |= gps_cflag_south;
        }
        else
        {
          /* Invalid; Bail. */
          gps_state = gps_state_null;
          return;
        }

        break;

      case gps_state_lon_dir:
        if (c == 'E')
        {
          gps_data.flags |= gps_cflag_east;
        }
        else if (c == 'W')
        {
          gps_data.flags |= gps_cflag_west;
        }
        else
        {
          /* Invalid; Bail. */
          gps_state = gps_state_null;
          return;
        }

        break;

      case gps_state_alt_u:
        if (c != 'M')
        {
          gps_state = gps_state_null;
          return;
        }

        break;
    }
  }

  gps_substate++;
}

/* This macro makes things easier in the select below. */
#define gps_store_bytes(dest_name)                                            \
                    case gps_state_ ## dest_name:                             \
                         gps_storing_location = gps_data.dest_name;           \
                         gps_storing_maxlen   = sizeof(gps_data.dest_name);   \
                         break;

void gps_next_field()
{
  /* Don't bother checking if its null, could be anything at null state */
  if (gps_state            != gps_state_null   && 
      gps_storing_maxlen   != 0                &&
      gps_substate         != gps_storing_maxlen)
  {
      /* We didn't fill the field properly. Discard it */
      gps_state = gps_state_null;
      return;
  }

  /* Some fields require an extra end-check */
  switch (gps_state)
  {
    case gps_state_sentence_name:
      if (gps_substate != gps_sentence_name_length)
      {
        /* No match. */
        gps_state = gps_state_null;
        return;
      } 

      break;

    case gps_state_lat_dir:
    case gps_state_lon_dir:
    case gps_state_alt_u:
      if (gps_substate != 1)
      {
        /* Something is wrong; bail. */
        gps_state = gps_state_null;
        return;
      }

      break;
  }

  gps_state++;
  gps_substate = 0;

  /* Find out if it is a simple store or not */
  switch (gps_state)
  {
    gps_store_bytes(time)
    gps_store_bytes(lat_d)
    gps_store_bytes(lat_m)
    gps_store_bytes(lat_s)
    gps_store_bytes(lon_d)
    gps_store_bytes(lon_m)
    gps_store_bytes(lon_s)
    gps_store_bytes(satc)
    gps_store_bytes(alt)

    default:
      gps_storing_maxlen = 0;
      break;
  }
}

void gps_init()
{
  /* TODO  (re)initialise the UART */
  /* Disable TX, that will be connected to the phone */

  gps_state = gps_state_null;
}

