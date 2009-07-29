/*
    Copyright (C) 2008  Daniel Richman & Simrun Basuita

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

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

/* 
   Atmega162:
   SS   - PB4  (Master Output)
   MOSI - PB5  (Master Output)
   MISO - PB6  (Master Input)
   SCK  - PB7  (Master Output) 

   Arduino-168:
   SS   - PB2  (Master Output)
   MOSI - PB3  (Master Output)
   MISO - PB4  (Master Input)
   SCK  - PB5  (Master Output) 

   Therefore we must translate our atmega162 code into 168 code by
   overriding the PB? macros to change those pins. Everything else
   is compatible between 162 and 168.

   The IO headers are included above and so won't be re-included, 
   and so won't overwrite our defines.
*/

#undef PB4
#undef PB5
#undef PB6
#undef PB7

#define PB4   2
#define PB5   3
#define PB6   4
#define PB7   5

#include "../../alien1/atmega162/tests/logtest.c"
