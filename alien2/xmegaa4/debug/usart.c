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

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "../data.h"
#include "buffer.h"
#include "usart.h"

#if DEBUG

static uint8_t usart_idle;

ISR(USARTD1_DRE_vect)
{
    uint8_t b, status;
    status = buffer_read_byte(&b);

    if (status == DATA_SOURCE_OK)
    {
        USARTD1.DATA = b;
    }
    else
    {
        usart_tx_disable();
    }
}

void usart_tx_enable()
{
    if (usart_idle == 0)
    {
        return;
    }

    USARTD1.CTRLA = USART_DREINTLVL_LO_gc;
    usart_idle = 0;
}

void usart_tx_disable()
{
    if (usart_idle == 1)
    {
        return;
    }

    USARTD1.CTRLA = 0;
    usart_idle = 1;
}

void usart_init()
{
    PORTD.DIRSET = 0x80;

    USARTD1.CTRLC = USART_CHSIZE_8BIT_gc;
    USARTD1.CTRLB = USART_TXEN_bm;

    /*
     * 9600 baud, BSCALE: -6, BSEL: 3269 
     * USARTD1.BAUDCTRLA = 197;
     * USARTD1.BAUDCTRLB = 172;
     */

    /* 115200 baud, BSCALE: -6, BSEL: 214 */
    USARTD1.BAUDCTRLA = 214;
    USARTD1.BAUDCTRLB = 160;

    usart_idle = 1;
}

#endif
