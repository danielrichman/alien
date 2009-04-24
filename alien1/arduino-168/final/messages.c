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
#define message_send_field_checksum    9
#define message_send_field_nl          10
#define message_send_field_end         11

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

/* Raw scopy */
#define   scopys(source, len, type)   scopy_len  = len;                   \
                                      scopy_src  = source;                \
                                      scopy_type = type;

/* Derivatives */
#define scopysba(source, len, type)  scopys(ba(source), len, type)

#define    scopy(source, type)       scopys(   source,  sizeof(source), type)
#define  scopyba(source, type)       scopys(ba(source), sizeof(source), type)

/* scopy types */
#define scopy_type_null     0x00
#define scopy_type_field    0x01
#define scopy_type_fstate   0x02
#define scopy_type_hexdump  0x04

/* Gets the next character to send */
uint8_t messages_get_char(payload_message *data, uint8_t message_type)
{
  uint8_t c;       /* Char to return     */
  uint8_t i;       /* Temporary variable */
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
      scopyba(data->system_location.fix_age, scopy_type_hexdump)
      break;

    case message_send_field_temperature:
      scopyba(data->system_temp, scopy_type_hexdump)
      break;

    case message_send_field_sysdata:
      if (message_type == message_type_log)
      {
        scopyba(data->system_state, scopy_type_hexdump)
        break;
      }

      /* else: break isn't called, so runs onto here */
      /* Skip this field when not writing to SD card log */
      data->message_send_field++;
      /* Don't break, move straight onto the next one... */

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
    /* This should never happen. It's kinda damage-limitation */
    c = '!';   /* Shouldn't == 0 if we haven't returned already */
  }

  if (data->message_send_field < message_send_field_checksum &&
      !(data->message_send_field == message_send_field_header &&
        data->message_send_fsubstate < 2))
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

  latest_data.message_id++;
}

