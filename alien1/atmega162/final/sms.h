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

#ifndef ALIEN_SMS_HEADER
#define ALIEN_SMS_HEADER

#include <stdint.h>

extern uint8_t sms_state, sms_mode;

#define sms_state_formatcmd     0
#define sms_state_cmdstart      1
#define sms_state_hexstart_a    2
#define sms_state_hexstart_b    3
#define sms_state_messagehex_a  4
#define sms_state_messagehex_b  5
#define sms_state_cmdend        6
#define sms_state_end           7

#define sms_mode_null           0
#define sms_mode_rts            1   /* request to send */
#define sms_mode_waiting        2
#define sms_mode_ready          3
#define sms_mode_busy           4

void sms_wait();
void sms_start();
void sms_init();

#endif 
