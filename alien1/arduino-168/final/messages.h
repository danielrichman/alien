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

#ifndef ALIEN_MESSAGES_HEADER
#define ALIEN_MESSAGES_HEADER

#include <stdint.h>
#include <stdlib.h>

/* $$A1,<INCREMENTAL COUNTER ID>,<TIME HH:MM:SS>,<N-LATITUDE DD.DDDDDD>,
 * <E-LONGITUDE DDD.DDDDDD>,<ALTITUDE METERS MMMMM>,<GPS_FIX_AGE_HEXDUMP>,
 * <GPS_SAT_COUNT>,<TEMPERATURE_HEXDUMP>,*<CHECKSUM><NEWLINE> */

/* GPS data struct */
#define gps_cflag_north  0x01
#define gps_cflag_south  0x02
#define gps_cflag_east   0x04
#define gps_cflag_west   0x08

typedef struct
{
  uint8_t   time[6];
  uint8_t  lat_d[2];  /* Latitude degrees */
  uint8_t  lat_p[6];  /* Latitude decimal-degrees */
  uint8_t  lon_d[3];  /* Same, for longitude */
  uint8_t  lon_p[6];
  uint8_t   satc[2];
  uint8_t    alt[5];
  uint8_t  flags;     /* Expresses if it's N/S and E/W; 4 LSB ONLY! */
  uint16_t fix_age;   /* How old the fix is, in seconds             */
} gps_information; 

/* Temperature data struct */
typedef struct
{
  uint16_t internal_temperature;
  uint16_t external_temperature;
} temperature_data;

/* Message structure */
typedef struct
{
  uint16_t message_id;              /* <INCREMENTAL COUNTER ID>
				     * will rendered into ascii-base10 */
  gps_information system_location;  /* Is already in ASCII, except 
				     * for the 'flags' field */
  temperature_data system_temp;     /* Hexdump this */

  uint8_t message_send_field;       /* These help out the message.c */
  uint8_t message_send_fstate;      /* get_char routines */
  uint8_t message_send_fsubstate;
  uint8_t message_send_checksum;
} payload_message;

/* Message Buffers; in order of freshness */
extern payload_message latest_data;  /* Where the next update is built & 
                                      * kept until ready*/
extern payload_message    log_data;  /* Logged whenever we get a full update */
extern payload_message  radio_data;  /* Copied from log data whenever 
                                      * the radio is ready */
extern payload_message    sms_data;  /* Sent very rarely */

/* Prototypes */
uint8_t messages_get_char(payload_message *data);
void messages_push();

#endif 

