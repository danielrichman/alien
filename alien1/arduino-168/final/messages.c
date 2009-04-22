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

#include "gps.h"  
#include "hexdump.h"
#include "messages.h"  
#include "radio.h" 
#include "sms.h"
#include "temperature.h"  
#include "timer1.h"

/* $$A1,<INCREMENTAL COUNTER ID>,<TIME HH:MM:SS>,<N-LATITUDE DD.DDDDDD>,
 * <E-LONGITUDE DDD.DDDDDD>,<ALTITUDE METERS MMMMM>,<GPS_FIX_AGE_HEXDUMP>,
 * <SYSTEM_STATE_DATA_HEXDUMP>,<PAYLOAD_MSG_ST_HXDMP_4LSB>
 * <NEWLINE> */

/* Message Buffers: see messages.h for more info */
payload_message latest_data, log_data, radio_data, sms_data;

/* payload_message.message_send_field */
#define message_send_field_header      0
#define message_send_field_inccnt      1
#define message_send_field_gpstime     2
#define message_send_field_gpslat      3
#define message_send_field_gpslon      4
#define message_send_field_gpsalt      5   /* Ukhas protocol finishes here */
#define message_send_field_gpsfixage   6
#define message_send_field_temperature 7
#define message_send_field_sysdata     8   /* skip for radio/sms */
#define message_send_field_nl          9
#define message_send_field_end         10

/* payload_message.message_send_fstate */
#define message_send_fstate_gpstime_hh      0
#define message_send_fstate_gpstime_mm      1
#define message_send_fstate_gpstime_ss      2
#define message_send_fstate_gpslat_sign     0
#define message_send_fstate_gpslat_d        1
#define message_send_fstate_gpslat_p        2
#define message_send_fstate_gpslon_sign     0
#define message_send_fstate_gpslon_d        1
#define message_send_fstate_gpslon_p        2

/* fcname: flight computer name, name of our balloon */
#define message_header_length               4
uint8_t message_header[message_header_length] = { '$', '$', 'A', '1' };

/* powten: look up table for 10 to the power n */
uint16_t powten[5] = { 1, 10, 100, 1000, 10000 };

/* Utility macros */
#define field_copys(source, length)                                          \
           if (data->message_send_fsubstate == (length))                     \
           {                                                                 \
             c = ',';                                                        \
             data->message_send_field++;                                     \
             data->message_send_fstate = 0;                                  \
             data->message_send_fsubstate = 0;                               \
           }                                                                 \
           else                                                              \
           {                                                                 \
             c = (source)[data->message_send_fsubstate];                     \
             data->message_send_fsubstate++;                                 \
           }

#define fstate_copys(source, length, delim)                                  \
           if (data->message_send_fsubstate == (length))                     \
           {                                                                 \
             c = delim;                                                      \
             data->message_send_fstate++;                                    \
             data->message_send_fsubstate = 0;                               \
           }                                                                 \
           else                                                              \
           {                                                                 \
             c = (source)[data->message_send_fsubstate];                     \
             data->message_send_fsubstate++;                                 \
           }

/* Casts anything like a struct to something we can hexdump/treat
 * as byte-array */
#define ba(source)  ( (uint8_t *) &source )

#define field_hexdumps(source, length)                                       \
           if (data->message_send_fsubstate == (length))                     \
           {                                                                 \
             c = ',';                                                        \
             data->message_send_field++;                                     \
             data->message_send_fstate = 0;                                  \
             data->message_send_fsubstate = 0;                               \
           }                                                                 \
           else                                                              \
           {                                                                 \
             if (data->message_send_fstate == 0)                             \
             {                                                               \
               j =  last_four( (ba(source))[data->message_send_fsubstate] ); \
               data->message_send_fstate = 1;                                \
             }                                                               \
             else                                                            \
             {                                                               \
               j = first_four( (ba(source))[data->message_send_fsubstate] ); \
               data->message_send_fstate = 0;                                \
               data->message_send_fsubstate++;                               \
             }                                                               \
                                                                             \
             c = num_to_char(j);                                             \
           }

#define    field_copy(source)           field_copys((source), sizeof(source))
#define   fstate_copy(source, delim)                                         \
                              fstate_copys((source), sizeof(source), (delim))
#define field_hexdump(source)        field_hexdumps((source), sizeof(source))

/* Gets the next character to send */
uint8_t messages_get_char(payload_message *data, uint8_t message_type)
{
  uint8_t c, e;    /* Char to return, is it the end? */
  uint8_t i, j;    /* Temporary variables */
  div_t divbuf;    /* Temporary divide result storage */

  /* Initialise to false */
  e = 0;
  c = 0;

  switch (data->message_send_field)
  {
    case message_send_field_header:
      field_copy(message_header)
      break;

    case message_send_field_inccnt:
      /* Modified integer to ascii follows */
      if (data->message_send_fstate == 0)
      {
        data->message_send_fsubstate = 4;
        data->message_send_fstate    = 1;
      }

      if (data->message_send_fsubstate == 0)
      {
        /* Should just be the units left */
        c = '0' + data->message_id;
        data->message_send_field++;
        data->message_send_fstate = 0;
        data->message_send_fsubstate = 0;
        break;
      }
      else
      {
        i = powten[data->message_send_fsubstate];
      }

      /* Time for some magic. Divide i into message_id *
       * gets stored in message_id */
      divbuf = div(data->message_id, i);

      /* Quotient is the current digit:
       * Start at '0' and count up to '1' ... */
      c = '0' + divbuf.quot;

      /* Remainder gets stored in message_id for the next pass */
      data->message_id = divbuf.rem;

      /* Move onto a lesser digit */
      data->message_send_fsubstate--;

      break;

    case message_send_field_gpstime:
      /* fstate_copy only increments fstate at the end, and allows a extra
       * delimiter (':') to be added. field copy finishes the time field off
       * and moves onto latitude */
      switch (data->message_send_fstate)
      {
        case message_send_fstate_gpstime_hh:
          fstate_copys(    data->system_location.time     , 2, ':')
          break;
        case message_send_fstate_gpstime_mm:
          fstate_copys( &((data->system_location.time)[2]), 2, ':')
          break;
        case message_send_fstate_gpstime_ss:
          field_copys(  &((data->system_location.time)[4]), 2)
          break;
      }

      break;

    case message_send_field_gpslat:
      switch (data->message_send_fstate)
      {
        case message_send_fstate_gpslat_sign:
          if (data->system_location.flags & gps_cflag_south)
          {
            c = '-';   /* Negative sign for southern latitude */
            data->message_send_fstate++;
            break;
          }

          /* Otherwise, move onto the next one */
          data->message_send_fstate++;
          /* no break - move straight on */

        case message_send_fstate_gpslat_d:
          fstate_copy(data->system_location.lat_d, '.')
          break;

        case message_send_fstate_gpslat_p:
          field_copy(data->system_location.lat_p)
          break;
      }
      break;

    case message_send_field_gpslon:
      switch (data->message_send_fstate)
      {
        case message_send_fstate_gpslon_sign:
          if (data->system_location.flags & gps_cflag_west)
          {
            c = '-';   /* Negative sign for western longitude */
            data->message_send_fstate++;
            break;
          }

          /* Otherwise, move onto the next one */
          data->message_send_fstate++;
          /* no break - move straight on */

        case message_send_fstate_gpslon_d:
          fstate_copy(data->system_location.lon_d, '.')
          break;

        case message_send_fstate_gpslon_p:
          field_copy(data->system_location.lon_p)
          break;
      }
      break;

    case message_send_field_gpsalt:
      field_copy(data->system_location.alt);
      break;

    case message_send_field_gpsfixage:
      field_hexdump(data->system_location.fix_age);
      break;

    case message_send_field_temperature:
      field_hexdump(data->system_temp);
      break;

    case message_send_field_sysdata:
      if (message_type == message_type_log)
      {
        field_hexdump(data->system_state);
        break;
      }

      /* else: break isn't called, so runs onto here */
      /* Skip this field when not writing to SD card log */
      data->message_send_field++;
      /* Don't break, move straight onto the next one... */

    case message_send_field_nl:
      c = '\n';
      data->message_send_field++;
      break;

    case message_send_field_end:
    default:
      e = 1;  /* Tells parent function that it is the end of transmission */
      break;
  }

  if (e == 1)
  {
    return 0;  /* Returning 0 ends the transmission */
  }

  if (c == 0)
  {
    c = '?';   /* Shouldn't == 0 if e != 1 */
  }

  return c;
}

/* Called every second, a signal to push the data onwards */
void messages_push()
{
  /* TODO Implement messages_push() */
  if (radio_state == radio_state_not_txing)
  {
    radio_data = latest_data;   /* Update the radio's copy */
    radio_send();               /* Go go go! */
  }

  if (/* TODO: if sms.c wants a message */0)
  {
    sms_data = latest_data;
    latest_data.system_state.SMSes_sent++;
    /* TODO: initiate sending process */
  }

  if (/* log.c is ready for a message */0)
  {
    log_data = latest_data;
    latest_data.system_state.data_lines_logged++;
    /* TODO: intiate logging process */
  }
}

