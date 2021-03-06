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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Prototypes */
void gps_data_push();
void gps_proc_byte(uint8_t c);
void gps_next_field();
int main(int argc, char **argv);

/* TODO: PUT THIS IN A HEADER FILE ########################################## */
/* FROM hexdump.c */
#define num_to_char(number)   (number < 10 ?                           \
                                      ('0' + number):                  \
                                      (('A' - 10) + number) )
#define first_four(byte)       (0x0F & byte)
#define  last_four(byte)      ((0xF0 & byte) >> 4)
/* TODO END ################################################################# */

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

/* GPS data struct */
#define gps_cflag_north  0x01
#define gps_cflag_south  0x02
#define gps_cflag_east   0x04
#define gps_cflag_west   0x08

typedef struct 
{
  uint8_t   time[6];
  uint8_t  lat_d[2];  /* Latitude degrees */
  uint8_t  lat_m[2];  /* Latitude minutes */
  uint8_t  lat_s[4];  /* Latitude part-minutes; ie. data after '.' */
  uint8_t  lon_d[3];  /* Same, for longitude */
  uint8_t  lon_m[2];
  uint8_t  lon_s[4];
  uint8_t  flags;     /* Expresses if it's N/S and E/W; 4 LSB ONLY! */
  uint8_t   satc[2];
  uint8_t    alt[5];
} gps_information;

/* Working data location - while we're recieving it goes here. */
gps_information gps_data;

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
      /* !!!! We've got valid data! woooot! */
      gps_data_push();

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

void gps_data_push()
{
  /* TODO DEBUG TODO */
  uint32_t i;
  uint8_t  flag_one, flag_two, flag_three, flag_four;

  #define dumpb(data_name)                                   \
       printf(#data_name ": ");                              \
       for (i = 0; i < sizeof(gps_data.data_name); i++)      \
         printf("%c", gps_data.data_name[i]);                \
       printf("\n");                                         

  dumpb(time)
  dumpb(lat_d)
  dumpb(lat_m)
  dumpb(lat_s)
  dumpb(lon_d)
  dumpb(lon_m)
  dumpb(lon_s)

  flag_one   = '-';
  flag_two   = '-';
  flag_three = '-';
  flag_four  = '-';

  if (gps_data.flags & gps_cflag_north) flag_one = 'N';
  if (gps_data.flags & gps_cflag_south) flag_two = 'S';

  if (gps_data.flags & gps_cflag_east) flag_three = 'E';
  if (gps_data.flags & gps_cflag_west) flag_four  = 'W';

  printf("flags: %.1X %c%c%c%c\n", gps_data.flags, 
                 flag_one, flag_two, flag_three, flag_four);

  dumpb(satc)
  dumpb(alt)
  /* END DEBUG TODO */
}

int main(int argc, char **argv)
{
  uint8_t c;

  c = getchar();
  while (c > 0 && c < 128)
  {
    gps_proc_byte(c);
    c = getchar();
  }

  return 0;
}
