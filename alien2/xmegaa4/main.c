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

#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "radio/radio.h"

static void clock_init();
static void interrupt_enable();
static void sleep_forever();

int main()
{
    clock_init();
    radio_init();
    interrupt_enable();
    sleep_forever();

    return 0;
}

static void clock_init()
{
    /* Setup the external osc for 8MHz with a large warmup time */
    OSC.XOSCCTRL = OSC_FRQRANGE_9TO12_gc | OSC_XOSCSEL_XTAL_16KCLK_gc;

    /* Enable the external osc in addition to the current osc */
    OSC.CTRL |= OSC_XOSCEN_bm;

    /* Wait until it is ready */
    while (!(OSC.STATUS & OSC_XOSCRDY_bm));

    /* Select the external osc */
    CCP = CCP_IOREG_gc;
    CLK.CTRL = CLK_SCLKSEL_XOSC_gc;

    /* Disable other oscs */
    OSC.CTRL = OSC_XOSCEN_bm;
}

static void interrupt_enable()
{
    PMIC.CTRL = PMIC_HILVLEN_bm;
    sei();
}

static void sleep_forever()
{
    for (;;) sleep_mode();
}
