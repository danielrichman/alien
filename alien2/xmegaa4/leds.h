/*
    Copyright (C) 2011  Daniel Richman

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

#ifndef __LEDS_H__
#define __LEDS_H__

#include <stdlib.h>
#include <avr/io.h>

/*
 * LED ONE: PORT A 4
 * LED TWO: PORT D 1
 */

#define LED_ONE_MASK (1 << 4)
#define LED_ONE_PORT PORTA
#define LED_TWO_MASK (1 << 1)
#define LED_TWO_PORT PORTD

#define LED_PORT(led) led ## _PORT
#define LED_MASK(led) led ## _MASK

#define led_set(led) \
    do \
    { \
        LED_PORT(led).DIRSET = LED_MASK(led); \
        LED_PORT(led).OUTSET = LED_MASK(led); \
    } \
    while (0)

#define led_clr(led) \
    do \
    { \
        LED_PORT(led).DIRCLR = LED_MASK(led); \
        LED_PORT(led).OUTCLR = LED_MASK(led); \
    } \
    while (0)

#define led_tgl(led) \
    do \
    { \
        LED_PORT(led).DIRTGL = LED_MASK(led); \
        LED_PORT(led).OUTTGL = LED_MASK(led); \
    } \
    while (0)

/* e.g., usage: led_set(LED_ONE) */

#endif
