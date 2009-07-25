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
#include <stdlib.h>

#include "camera.h"
#include "gps.h"
#include "hexdump.h"
#include "log.h"
#include "main.h"
#include "messages.h"
#include "radio.h"
#include "sms.h"
#include "statusled.h"
#include "temperature.h"
#include "timer1.h"
#include "timer3.h"
#include "watchdog.h"

/* A list of fields and their index, starting from 1. The index goes up
 * every time a ',' or a '.' is encountered, and it also goes up to separate
 * the degrees and minutes (manual code below) */
#define gps_state_null            0    /* Invalidated status, waiting for $ */
#define gps_state_sentence_name   1    /* Eg GPGGA */
#define gps_state_time            2
#define gps_state_lat_d           4
#define gps_state_lat_p           5
#define gps_state_lat_pp          6
#define gps_state_lat_dir         7
#define gps_state_lon_d           8
#define gps_state_lon_p           9
#define gps_state_lon_pp          10
#define gps_state_lon_dir         11
#define gps_state_satc            13
#define gps_state_alt             16
#define gps_state_alt_u           17
#define gps_state_checksum        21

/* substate is used for counting through each uint8_t */
uint8_t gps_state, gps_checksum, gps_substate, gps_storing_maxlen, gps_prem;
uint8_t *gps_storing_location;

/* Set to 5 when $GPGGA is matched. Decreased otherwise each second */
uint8_t gps_rx_ok;

/* GPGGA sentences provide fix data */
uint8_t gps_sentence_mask[5] = { 'G', 'P', 'G', 'G', 'A' };

/* Working data location - while we're recieving it goes here. */
gps_information gps_data;

ISR (USART0_RXC_vect)
{
  uint8_t i, j;  /* General purpose temporary variables, used in the for loop
                    that clears gps_data and the checksum checks.
                    Should be optimised out. */
  uint8_t c;     /* We store the char that we have just recieved here. */
  div_t divbuf;  /* Temporary divide result storage */

  /* Grab the character from the data register */
  c = UDR0;

  /* Reset the idle counter */
  timer1_uart_idle_counter = 0;

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

    /* Reset the gps_data struct, This will also reset the fix_age variable */
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
      latest_data.system_location = gps_data;
      /* The fix_age will have been overwritten with 0      */

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
        /* If it's too long or the wrong char... */
        if ((gps_substate == sizeof(gps_sentence_mask)) ||
            (c != gps_sentence_mask[gps_substate]))
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

      case gps_state_lat_p:
      case gps_state_lat_pp:
      case gps_state_lon_p:
      case gps_state_lon_pp:
        /* Digit = c - '0'. Add it to the carry/remainder */
        gps_prem += c - '0';

        /* Now divide by six */
        divbuf = udiv(gps_prem, 6);

        if (gps_substate == 0)
        {
          if (divbuf.quot != 0)
          {
            /* If this is digit 0, and divbuf.quot != 0, something is wrong
             * (there's more than 60 minutes !?) */
            gps_state = gps_state_null;
            return;
          }
        }
        else
        {
          /* Quotient is the current output digit. Store it.
           * Because we're also multiplying by ten 
           * [ie. output = (lat_p / 6) * 10 ] to convert decimal minutes
           * to decimal degrees, we store it in the previous char (ie, 
           * shift decimal digits left is *10 ) */
          gps_storing_location[gps_substate - 1] = '0' + divbuf.quot;
        }

        /* Remainder goes back into prem, multiplied by ten 
         * (when we're on the next digit this one is 10 times bigger) */
        gps_prem = divbuf.rem * 10;

        if (gps_substate == 5)
        {
          /* We'll have finished recieving input by now, but we 
           * still have a remainder and still have the last char
           * to fill. Treat the last char as 0, so nothing to add to
           * prem. Now we divide and just stuff the quotient in */
          divbuf = udiv(gps_prem, 6);
          gps_storing_location[5] = '0' + divbuf.quot;
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
  /* Don't bother checking if it's null, could be anything at null state */
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
      if (gps_substate != sizeof(gps_sentence_mask))
      {
        /* No match. */
        gps_state = gps_state_null;
        return;
      } 

      /* Good match */
      gps_rx_ok = 5;

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

    case gps_state_lat_pp:
    case gps_state_lon_pp:
      if (gps_substate != 6)
      {
        /* Field not filled, calculation not complete. */
        gps_state = gps_state_null;
        return;
      }

      break;
  }

  /* We don't reset substate for lat/lon_p, as we need to maintain
   * that field through to _pp */
  if (gps_state != gps_state_lat_p && gps_state != gps_state_lon_p)
  {
    gps_substate = 0;
  }

  gps_state++;
  gps_storing_maxlen = 0;  /* Defaults to zero */

  /* Find out if it is a simple store or not */
  switch (gps_state)
  {
    gps_store_bytes(time)
    gps_store_bytes(lat_d)
    gps_store_bytes(lon_d)
    gps_store_bytes(satc)
    gps_store_bytes(alt)

    /* lat_p and lon_p (and lat_pp and lon_pp) share the same special-
     * processing code. We use the gps_storing_location as a way of choosing
     * between lat and lon inside that shared code without big if statements.
     * However, we do not enable simple store by gps_storing_maxlen */

    case gps_state_lat_p:
      gps_prem = 0;     /* This holds the remainder/carry for the next digit */
      /* Don't break, line below applies too */

    case gps_state_lat_pp:
      gps_storing_location = gps_data.lat_p;
      break;

    case gps_state_lon_p:
      gps_prem = 0;
      /* Don't break, line below applies too */

    case gps_state_lon_pp:
      gps_storing_location = gps_data.lon_p;
      break;
  }
}

void gps_init()
{
  gps_state = gps_state_null;

  /* UBRR = F_CPU/(16 * baudrate) - 1 
   *      = 16000000/16b - 1
   *      = 1000000/b - 1
   *      = 1000000/4800 - 1 = 207.3333 */
  UBRR0L = 207;

  /* Enable Recieve Interrupts and UART RX mode. Don't enable TX */
  UCSR0B = ((_BV(RXCIE0)) | (_BV(RXEN0)));
}

