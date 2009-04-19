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
#include "radio.h"

/* $$alien1,<INCREMENTAL COUNTER ID>,<TIME HH:MM:SS>,<LATITUDE DD.DDDDDD>,
 * <LONGITUDE DD.DDDDDD>,<ALTITUDE METERS MMMMM>,<GPS_FLAGS_HEXDUMP>,
 * <PAYLOAD_STATE_DATA_HEXDUMP><NEWLINE> */

/* Message Buffers: see messages.h for more info */
payload_message latest_data, log_data, radio_data, sms_data;

/* Called when there is new data in the extern gps_data */
void messages_gps_data_push()
{
  /* TODO Implement me */
}

/* Gets the next character to send */
uint8_t messages_get_char(payload_message *data, uint8_t message_type)
{
  /* TODO Implement me */
  return '!';

  /* Return 0 if there is nothing more to send:
   * return 0; */
  /* Note: if (message_type == message_type_radio|sms) then DON'T
   * send system_state. It's just not needed on the radio/SMS  */
}

/* Initialises the Message Buffers */
void messages_init()
{
  /* TODO Implement me */
}

