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

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "messages.h"
#include "hexdump.h"
#include "log.h"
#include "main.h"
#include "radio.h"
#include "sms.h"

/* NOTE: messages.h has a hardcoded max-length for messages, which must be 
 * kept up to date! */

/* $$A1,<INCREMENTAL COUNTER ID>,<TIME HH:MM:SS>,<N-LATITUDE DD.DDDDDD>,
 * <E-LONGITUDE DDD.DDDDDD>,<ALTITUDE METERS MMMMM>,<GPS_FIX_AGE_HEXDUMP>,
 * <GPS_SAT_COUNT>,<TEMPERATURE_HEXDUMP>,<MCUCSR,GPS_RX_OK HEXDUMP>
 * *<CHECKSUM><NEWLINE> */

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
#define message_send_field_gpssatc     7
#define message_send_field_temperature 8
#define message_send_field_state       9
#define message_send_field_checksum    10
#define message_send_field_nl          11
#define message_send_field_end         12

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
uint8_t message_header[4] = { '$', '$', 'A', '1' };

/* powten: look up table for 10 to the power n */
uint16_t powten[5] = { 1, 10, 100, 1000, 10000 };

/* Raw scopy */
#define   scopys(source, len, type)   scopy_len  = len;                   \
                                      scopy_src  = source;                \
                                      scopy_type = type;

/* Derivatives */
#define scopysba(source, len, type)  scopys(ba(source), len, type)

#define    scopy(source, type)       scopys(   source,  sizeof(source), type)
#define  scopyba(source, type)       scopys(ba(source), sizeof(source), type)

/* scopy types */
#define scopy_type_null        0x00
#define scopy_type_field       0x01
#define scopy_type_fstate      0x02
#define scopy_type_hexdump     0x04

/* Gets the next character to send */
uint8_t messages_get_char(payload_message *data)
{
  uint8_t c;       /* Char to return */
  uint8_t t;       /* Another Temporary Variable */
  div_t divbuf;    /* Temporary divide result storage */

  /* scopy enabling values */
  uint8_t scopy_len, scopy_type, scopy_delim;
  uint8_t *scopy_src;

  /* Initialise to false */
  c = 0;
  scopy_type  = 0;
  scopy_delim = 1;

  switch (data->message_send_field)
  {
    case message_send_field_header:
      scopy(message_header, scopy_type_field)
      break;

    case message_send_field_inccnt:
      /* Modified integer to ascii follows */
      if (data->message_send_fstate == 0)
      {
        data->message_send_fsubstate = 4;
        data->message_send_fstate    = 1;
      }

      if (data->message_send_fstate == 2)
      {
        c = ',';
        data->message_send_field++;
        data->message_send_fstate = 0;
        data->message_send_fsubstate = 0;
        break;
      }

      /* Time for some magic. Divide 10^n into message_id *
       * gets stored in message_id */
      divbuf = udiv(data->message_id, powten[data->message_send_fsubstate]);

      /* Quotient is the current digit:
       * Start at '0' and count up to '1' ... */
      c = '0' + divbuf.quot;

      /* Remainder gets stored in message_id for the next pass */
      data->message_id = divbuf.rem;

      /* Move onto a lesser digit */
      if (data->message_send_fsubstate == 0)
      {
        /* Flag it as finished, need to send comma and move on */
        data->message_send_fstate = 2;
      }
      else
      {
        data->message_send_fsubstate--;
      }

      break;

    case message_send_field_gpstime:
      switch (data->message_send_fstate)
      {
        case message_send_fstate_gpstime_hh:
          scopys(    data->system_location.time     , 2, scopy_type_fstate)
          scopy_delim = ':';
          break;
        case message_send_fstate_gpstime_mm:
          scopys( &((data->system_location.time)[2]), 2, scopy_type_fstate)
          scopy_delim = ':';
          break;
        case message_send_fstate_gpstime_ss:
          scopys( &((data->system_location.time)[4]), 2, scopy_type_field)
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
          scopy(data->system_location.lat_d, scopy_type_fstate)
          scopy_delim = '.';
          break;

        case message_send_fstate_gpslat_p:
          scopy(data->system_location.lat_p, scopy_type_field)
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
          scopy(data->system_location.lon_d, scopy_type_fstate)
          scopy_delim = '.';
          break;

        case message_send_fstate_gpslon_p:
          scopy(data->system_location.lon_p, scopy_type_field)
          break;
      }
      break;

    case message_send_field_gpsalt:
      scopy(data->system_location.alt, scopy_type_field)
      break;

    case message_send_field_gpsfixage:
      /* Compensate for the endian-ness */
      if (data->message_send_fsubstate == 2)
      {
        c = ',';
        data->message_send_field++;
        data->message_send_fstate = 0;
        data->message_send_fsubstate = 0;
      }
      else
      {
        if (data->message_send_fsubstate == 0)
        {
          t = (data->system_fix_age & 0xFF00) >> 8;
        }
        else
        {
          t =  data->system_fix_age & 0x00FF;
        }

        if (data->message_send_fstate == 0)
        {
          c =  last_four(t);
          data->message_send_fstate = 1;
        }
        else
        {
          c = first_four(t);
          data->message_send_fstate = 0;
          data->message_send_fsubstate++;
        }

        c = num_to_char(c);
      }
      break;

    case message_send_field_gpssatc:
      scopy(data->system_location.satc, scopy_type_field)
      break;

    case message_send_field_temperature:
      scopyba(data->system_temp, scopy_type_hexdump)
      break;

    case message_send_field_state:
      scopyba(data->system_state, scopy_type_hexdump)
      break;

    case message_send_field_checksum:
      switch (data->message_send_fstate)
      {
        case 0:
          c = '*';
          data->message_send_fstate++;
          break;

        case 1:
          c = hexdump_a(data->message_send_checksum);
          data->message_send_fstate++;
          break;

        case 2:
          c = hexdump_b(data->message_send_checksum);
          data->message_send_field++;
          data->message_send_fstate = 0;
          break;
      }

      break;

    case message_send_field_nl:
      c = '\n';
      data->message_send_field++;
      break;

    case message_send_field_end:
    default:
      return 0; /* Tells parent function that it is the end of transmission */
      break;
  }

  if (scopy_type != scopy_type_null)
  {
    if (data->message_send_fsubstate == scopy_len) 
    {
      if (scopy_type == scopy_type_fstate)
      {
        c = scopy_delim;
        data->message_send_fstate++;
      }
      else
      {
        c = ',';
        data->message_send_field++; 
        data->message_send_fstate = 0;  
      }

      data->message_send_fsubstate = 0;
    } 
    else  
    {
      if (scopy_type == scopy_type_hexdump)
      {
        if (data->message_send_fstate == 0) 
        {
          c =  last_four( scopy_src[data->message_send_fsubstate] ); 
          data->message_send_fstate = 1;
        }
        else
        {
          c = first_four( scopy_src[data->message_send_fsubstate] ); 
          data->message_send_fstate = 0;
          data->message_send_fsubstate++;
        }

        c = num_to_char(c); 
      }
      else
      {
        c = scopy_src[data->message_send_fsubstate];
        data->message_send_fsubstate++;
      }
    }
  }

  if (c == 0)
  {
    /* Shouldn't == 0 if we haven't returned already. This may happen for the
     * first few messages where there is no gps data */
    c = '!';
  }

  if (data->message_send_field < message_send_field_checksum &&
      !(data->message_send_field == message_send_field_header &&
        data->message_send_fsubstate < 3))
  {
    /* If: the current char is before the checksum (we don't wanna check the
     * the * or the checksum itself) AND it's not the first two chars of 
     * the header (we exclude the $$) then compute a NMEA-style xor-checksum */

    data->message_send_checksum ^= c;
  }

  return c;
}

/* Called every second, a signal to push the data onwards */
void messages_push()
{
  if (radio_state == radio_state_not_txing)
  {
    /* Update the radio's copy, and begin transmission! */
    memcpy(&radio_data, &latest_data, sizeof(payload_message));
    radio_send();
  }

  if (log_state == log_state_initreset || log_state == log_state_idle)
  {
    memcpy(&log_data,   &latest_data, sizeof(payload_message));
    log_start();
  }

  if (sms_mode == sms_mode_data)
  {
    memcpy(&sms_data,   &latest_data, sizeof(payload_message));
    sms_mode = sms_mode_rts;
  }

  latest_data.message_id++;
}

