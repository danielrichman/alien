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

#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include "hardware.h"
#include "radio.h"

#define RADIO_HW_MODE_PORT  PORTA
#define RADIO_DAC           DACB
#define RADIO_HW_TIMER      TCC0

ISR (TCC0_OVF_vect)
{
    radio_isr();
}

void radio_hw_init()
{
    RADIO_DAC.CTRLA = DAC_CH0EN_bm | DAC_ENABLE_bm;
    RADIO_DAC.CTRLC = DAC_REFSEL_INT1V_gc;
}

void radio_hw_set_dac(uint16_t value)
{
    RADIO_DAC.CH0DATA = value;
}

/*
 * If we genuinely put the radio into idle then when we turned it back on
 * there would be a *massive* warmup time while it approaches the correct
 * frequency. This is a total pain and totally breaks hell & morse, and
 * produces an annoying warm up time for rtty and domex.
 */
#define IDLE_FREQ 4000

void radio_hw_set_mode(uint8_t mode)
{
    if (mode == RADIO_HW_MODE_IDLE)
    {
        mode = RADIO_HW_MODE_TX;
        radio_hw_set_dac(IDLE_FREQ);
    }

    RADIO_HW_MODE_PORT.DIRCLR = RADIO_HW_MODE_BITS;
    RADIO_HW_MODE_PORT.DIRSET = mode;
}

void radio_hw_set_speed(uint8_t div, uint16_t per)
{
    RADIO_HW_TIMER.CTRLA = 0;
    RADIO_HW_TIMER.CTRLFSET = TC_CMD_RESET_gc;
    RADIO_HW_TIMER.INTCTRLA = TC_OVFINTLVL_HI_gc;
    RADIO_HW_TIMER.PER = per;
    RADIO_HW_TIMER.CTRLA = div;
}

void radio_hw_queue_period_update(uint16_t per)
{
    RADIO_HW_TIMER.PERBUF = per;
}
