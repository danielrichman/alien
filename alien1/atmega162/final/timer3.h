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

#ifndef ALIEN_TIMER3_HEADER
#define ALIEN_TIMER3_HEADER

#include <stdint.h>

/* for sms.c and timers.c */
#define timer3_clear()   TCNT3  = 0;
#define timer3_start()   TCCR3B = ((_BV(CS32)) | (_BV(WGM32)));
#define timer3_stop()    TCCR3B = 0;

/* Prototype */
void timer3_init();

#endif 
