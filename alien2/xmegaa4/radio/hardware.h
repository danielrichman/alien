/*
    Copyright (C) 2010  Daniel Richman

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

#ifndef __RADIO_HARDWARE_H__
#define __RADIO_HARDWARE_H__

#include <stdint.h>
#include <avr/io.h>

#define RADIO_HW_TIMER_DIV1    TC_CLKSEL_DIV1_gc
#define RADIO_HW_TIMER_DIV2    TC_CLKSEL_DIV2_gc
#define RADIO_HW_TIMER_DIV4    TC_CLKSEL_DIV4_gc
#define RADIO_HW_TIMER_DIV8    TC_CLKSEL_DIV8_gc
#define RADIO_HW_TIMER_DIV64   TC_CLKSEL_DIV64_gc
#define RADIO_HW_TIMER_DIV256  TC_CLKSEL_DIV256_gc
#define RADIO_HW_TIMER_DIV1024 TC_CLKSEL_DIV1024_gc

#define RADIO_HW_MODE_IDLE  0
#define RADIO_HW_MODE_TXOFF 3
#define RADIO_HW_MODE_RX   (1 << 5)
#define RADIO_HW_MODE_TX   (1 << 6)
#define RADIO_HW_MODE_BITS (RADIO_HW_MODE_RX | RADIO_HW_MODE_TX)

void radio_hw_init();
void radio_hw_dac_set(uint16_t value);
void radio_hw_adc_get(uint16_t *af, uint16_t *rssi);
void radio_hw_timer_set(uint8_t div, uint16_t per);
void radio_hw_mode(uint8_t mode);
void radio_hw_queue_period_update(uint16_t per);

#endif
