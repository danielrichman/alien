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

#include "usart.h"
#include "debug.h"

#if DEBUG

static uint8_t debug_boot_message[] = "This is ALIEN-2\nDebug USART Ready\n\n";

void debug_init()
{
    usart_init();
    debug_write(debug_boot_message, sizeof(debug_boot_message));
}

uint8_t debug_write(uint8_t *data, uint16_t len)
{
    uint8_t status;
    status = buffer_write(data, len);
    usart_tx_enable();
    return status;
}

#endif
