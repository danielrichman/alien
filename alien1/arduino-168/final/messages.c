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
 * <LONGITUDE DD.DDDDDD>,<ALTITUDE METERS MMMMM>,<GPS_FLAGS_HEXDUMP_4LSB>,
 * <SYSTEM_STATE_DATA_HEXDUMP>,<PAYLOAD_MESSAGE_STATE_HEXDUMP_4LSB><NEWLINE> */

/* Message Buffers: see messages.h for more info */
payload_message latest_data, log_data, radio_data, sms_data;

/* Gets the next character to send */
uint8_t messages_get_char(payload_message *data, uint8_t message_type)
{
  /* TODO Implement me: messages_get_char() */
  return '!';

  /* Return 0 if there is nothing more to send:
   * return 0; */
  /* Note: if (message_type == message_type_radio|sms) then DON'T
   * send system_state. It's just not needed on the radio/SMS  */
}

/* Called every second, a signal to push the data onwards */
void messages_push()
{
  /* TODO Implement messages_push() */
}

/* Called on power up */
void messages_init()
{
  latest_data.message_status = message_status_stale;
}

