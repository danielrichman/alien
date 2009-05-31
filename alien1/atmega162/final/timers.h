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

#ifndef ALIEN_TIMERS_HEADER
#define ALIEN_TIMERS_HEADER

#include <stdint.h>

/* gps.c needs access to this */
extern uint8_t timers_uart_idle_counter;

/* for sms.c and timers.c */
#define timers_t3_clear        TCNT3  = 0;
#define timers_t3_start        TCCR3B = ((_BV(CS32)) | (_BV(WGM32)));
#define timers_t3_stop         TCCR3B = 0;
#define timers_is_t3_started   (TCCR3B == 0)

/* Prototype */
void timers_init();

#endif 
