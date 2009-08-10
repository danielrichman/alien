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

/* Hardcoded messages max length. Since the length of a message can vary, this
 * specifies the maximum, or a near-maximum. This _must_ be kept up-to-date! */
#define messages_max_length 75

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
} gps_information; 

/* Temperature data struct */
typedef struct
{
  uint8_t internal_msb;
  uint8_t internal_lsb;
  uint8_t external_msb;
  uint8_t external_lsb;
} temperature_data;

/* Message structure */
typedef struct
{
  uint16_t message_id;              /* <INCREMENTAL COUNTER ID>
				     * will rendered into ascii-base10 */
  gps_information system_location;  /* Is already in ASCII, except 
				     * for the 'flags' field */
  uint16_t system_fix_age;          /* How old the fix is, in seconds  */
  temperature_data system_temp;     /* Hexdump this */
  uint8_t system_state;             /* 7 - MCUCSR-WDT, 6 - log_ok, 
                                       3..0 - gps_rx_ok */
  uint8_t message_send_field;       /* These help out the message.c */
  uint8_t message_send_fstate;      /* get_char routines */
  uint8_t message_send_fsubstate;
  uint8_t message_send_checksum;
} payload_message;

#define messages_clear_gps_rx_ok()  latest_data.system_state &= ~(0x0F)
#define messages_set_gps_rx_ok(val)                                 \
                                    latest_data.system_state |= (0x0F & (val))
#define messages_get_gps_rx_ok()   (latest_data.system_state & 0x0F)

#define messages_set_log_ok()       latest_data.system_state |=  (0x40)
#define messages_clear_log_ok()     latest_data.system_state &= ~(0x40)
#define messages_get_log_ok()      (latest_data.system_state & 0x40)

#define messages_set_mcucsr_wdt()   latest_data.system_state |=  (0x80)
#define messages_clear_mcucsr_wdt() latest_data.system_state &= ~(0x80)

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

