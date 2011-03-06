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

#ifndef __DEBUG_USART_H__
#define __DEBUG_USART_H__

#include <stdint.h>
#include "debug.h"

#if DEBUG

void usart_tx_enable();
void usart_tx_disable();
void usart_init();

#endif

#endif
