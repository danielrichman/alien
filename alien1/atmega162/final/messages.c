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

/* NOTE: messages.c is expected to only use alphanumeric and .,:-$!* chars,
 * and a newline. The only problem char is the $ which has a different
 * code in the GSM alphabet (see sms.c) */

/* $$A1,<INCREMENTAL COUNTER ID>,<TIME HH:MM:SS>,<N-LATITUDE DD.DDDDDD>,
 * <E-LONGITUDE DDD.DDDDDD>,<ALTITUDE METERS MMMMM>,<GPS_FIX_AGE_HEXDUMP>,
 * <GPS_SAT_COUNT>,<TEMPERATURE_HEXDUMP>,<MCUCSR,GPS_RX_OK HEXDUMP>
 * *<CHECKSUM><NEWLINE> */

/* Message Buffers: see messages.h for more info */
payload_message latest_data, log_data, radio_data, sms_data;

/* payload_message.message_send_field */
#define message_send_field_flagstart_a 0
#define message_send_field_flagstart_b 1
#define message_send_field_header      2
#define message_send_field_inccnt      3
#define message_send_field_gpstime_hh  4
#define message_send_field_gpstime_mm  5
#define message_send_field_gpstime_ss  6
#define message_send_field_gpslat_sign 7
#define message_send_field_gpslat_d    8
#define message_send_field_gpslat_p    9
#define message_send_field_gpslon_sign 10
#define message_send_field_gpslon_d    11
#define message_send_field_gpslon_p    12
#define message_send_field_gpsalt      13  /* Ukhas protocol finishes here */
#define message_send_field_gpsfixage   14
#define message_send_field_gpssatc     15
#define message_send_field_temperature 16 
#define message_send_field_state       17
#define message_send_field_checksum    18
#define message_send_field_nl          19
#define message_send_field_end         20

/* fcname: flight computer name, name of our balloon */
uint8_t message_header[2] = { 'A', '1' };

/* powten: look up table for reverse powers of 10 */
uint16_t powten[5] = { 10000, 1000, 100, 10, 1 };

/* Scopy: Simple copy. */

/* Raw scopy */
#define   scopyr(source, length, type)                                    \
        (scopy_src = (source), scopy_length = (length), scopy_type = (type))

/* Derivatives */
#define    scopy(source)                                                  \
              scopyr(   source,  sizeof(source),     scopy_type_rawcopy)
#define   scopys(source, length)                                          \
              scopyr(   source,  length,             scopy_type_rawcopy)
#define  scopyhd(source)                                                  \
              scopyr(ba(source), sizeof(source) * 2, scopy_type_hexdump)

/* scopy types */
#define scopy_type_null        0x00
#define scopy_type_rawcopy     0x01
#define scopy_type_hexdump     0x02

/* Gets the next character to send */
uint8_t messages_get_char(payload_message *data)
{
  uint8_t c;               /* Char to return */
  uint8_t i, j, k, l, t;   /* Temporary Variables */
  div_t divbuf;            /* Temporary divide result storage */
  uint8_t field_delim;

  /* scopy enabling values */
  uint8_t *scopy_src;
  uint8_t  scopy_length;
  uint8_t  scopy_type;

  /* Initialise to false */
  c = 0;
  scopy_type = 0;

  /* By default each field is separated by a comma */
  field_delim = ',';

  switch (data->message_send_field)
  {
    case message_send_field_flagstart_a:
    case message_send_field_flagstart_b:
      c = '$';
      data->message_send_field++;
      break;

    case message_send_field_header:
      scopy(message_header);
      break;

    case message_send_field_inccnt:
      /* Modified integer to ascii follows */

      if (data->message_send_fsubstate == 5)
      {
        c = field_delim;
        data->message_send_field++;
        data->message_send_fsubstate = 0;
      }
      else
      {
        /* Time for some magic. Divide 10^n into message_id *
         * gets stored in message_id */
        divbuf = udiv(data->message_id, powten[data->message_send_fsubstate]);

        /* Quotient is the current digit:
         * Start at '0' and count up to '1' ... */
        c = '0' + divbuf.quot;

        /* Remainder gets stored in message_id for the next pass */
        data->message_id = divbuf.rem;

        /* Move onto a lesser digit (powten counts powers down) */
        data->message_send_fsubstate++;
      }

      break;

    case message_send_field_gpstime_hh:
      scopys(data->system_location.time    , 2);
      field_delim = ':';
      break;

    case message_send_field_gpstime_mm:
      scopys(data->system_location.time + 2, 2);
      field_delim = ':';
      break;

    case message_send_field_gpstime_ss:
      scopys(data->system_location.time + 4, 2);
      break;

    case message_send_field_gpslat_sign:
      /* Whatever happens, we'll be moving onto the next field */
      data->message_send_field++;

      if (data->system_location.flags & gps_cflag_south)
      {
         c = '-';   /* Negative sign for southern latitude */
         break;
      }

      /* Otherwise, fall through to the next field: no break */

    case message_send_field_gpslat_d:
      scopy(data->system_location.lat_d);
      field_delim = '.';
      break;

    case message_send_field_gpslat_p:
      scopy(data->system_location.lat_p);
      break;

    case message_send_field_gpslon_sign:
      data->message_send_fsubstate++;

      if (data->system_location.flags & gps_cflag_west)
      {
        c = '-';
        break;
      }

    case message_send_field_gpslon_d:
      scopy(data->system_location.lon_d);
      field_delim = '.';
      break;

    case message_send_field_gpslon_p:
      scopy(data->system_location.lon_p);
      break;

    case message_send_field_gpsalt:
      scopy(data->system_location.alt);
      break;

    case message_send_field_gpsfixage:
      /* Compensate for the endian-ness */
      if (data->message_send_fsubstate == 4)
      {
        c = field_delim;
        data->message_send_field++;
        data->message_send_fsubstate = 0;
      }
      else
      {
        if (data->message_send_fsubstate < 2)
        {
          t = (data->system_fix_age & 0xFF00) >> 8;
        }
        else
        {
          t =  data->system_fix_age & 0x00FF;
        }

        /* Is it even or odd? if bit 1 is clear it is even */
        if (!(data->message_send_fsubstate & 0x01))
        {
          i =  last_four(t);
        }
        else
        {
          i = first_four(t);
        }

        c = num_to_char(i);
        data->message_send_fsubstate++;
      }

      break;

    case message_send_field_gpssatc:
      scopy(data->system_location.satc);
      break;

    case message_send_field_temperature:
      scopyhd(data->system_temp);
      break;

    case message_send_field_state:
      scopyhd(data->system_state);
      field_delim = '*';   /* Checksum starts with a '*' */
      break;

    case message_send_field_checksum:
      if (data->message_send_fsubstate == 0)
      {
        l =  last_four(data->message_send_checksum);
	data->message_send_fsubstate++;
      }
      else
      {
        l = first_four(data->message_send_checksum);
        data->message_send_field++;
        data->message_send_fsubstate = 0;
      }

      c = num_to_char(l);
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
    /* The scopy macro will double scopy_len if it is in hexdump mode */
    if (data->message_send_fsubstate == scopy_length) 
    {
      c = field_delim;
      data->message_send_field++;
      data->message_send_fsubstate = 0;
    } 
    else  
    {
      if (scopy_type == scopy_type_hexdump)
      {
        /* value & 0x01 is true if value is odd, 
         * value >> 1 divides by two, discarding remainder */
        k = scopy_src[data->message_send_fsubstate >> 1];

        if (!(data->message_send_fsubstate & 0x01)) 
        {
          j =  last_four(k); 
        }
        else
        {
          j = first_four(k); 
        }

        c = num_to_char(j); 
        data->message_send_fsubstate++;
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

  if (data->message_send_field < message_send_field_checksum)
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

  if (log_state == log_state_initreset || log_state == log_state_datawait)
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

