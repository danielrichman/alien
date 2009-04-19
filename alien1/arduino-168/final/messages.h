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

#ifndef ALIEN1_MESSAGES_HEADER
#define ALIEN1_MESSAGES_HEADER

/* $$alien1,<INCREMENTAL COUNTER ID>,<TIME HH:MM:SS>,<LATITUDE DD.DDDDDD>,
 * <LONGITUDE DD.DDDDDD>,<ALTITUDE METERS MMMMM>,<GPS_FLAGS_HEXDUMP>,
 * <PAYLOAD_STATE_DATA_HEXDUMP><NEWLINE> */

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

/* Data string struct */
#define payload_status_ascending   0x01
#define payload_status_descending  0x02
#define payload_status_landed      0x04   /* ... etc. TODO */

typedef struct
{
  uint16_t payload_status;       /* Flying, descending, shots/sec, etc. */
  uint16_t internal_temperature;
  uint16_t external_temperature;
  uint16_t photos_taken;
  uint16_t SMSes_sent;
  uint16_t data_lines_logged;
} payload_state_data;

/* Message structure */
typedef struct
{
  uint16_t message_id;              /* <INCREMENTAL COUNTER ID>
				     * will rendered into ascii-base10 */
  gps_information system_location;  /* Is already in ASCII, except 
				     * for the 'flags' field */
  payload_state_data system_state;  /* Fully Hexdumped */

  uint8_t message_state;            /* This helps out the message.c */
  uint8_t message_substate;         /* a get_char routines */
} payload_message;

/* Message Buffers; in order of freshness */
extern payload_message   log_data;  /* Logged whenever we get a full update */
extern payload_message radio_data;  /* Copied from log data whenever 
                                     * the radio is ready */
extern payload_message   sms_data;  /* Sent very rarely */

/* Prototypes */
uint8_t messages_get_char(payload_message *data);
void messages_gps_data_push();      /* Called when GPS data is updated. */
void messages_init();
/* TODO More functions ;) */

#endif 

