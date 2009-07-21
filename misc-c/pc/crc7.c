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

#include <stdio.h>
#include <stdint.h>

uint8_t crc7_byte_update(uint8_t crc, uint8_t b)
{
  uint8_t i;

  for (i = 0x80; i != 0; i = i >> 1)
  {
    crc = crc << 1;

    if ((b & i) || (crc & 0x80))
    {
      if (!((b & i) && (crc & 0x80)))
      {
        crc ^= 0x09;
      }
    }
  }

  return crc & 0x7f;
}

#define crc7_finish(crc)   (((crc) << 1) | 0x01)

#define a(c, b) crc7_byte_update(c, b)
#define z(c)    crc7_finish(c)
#define f(d, e, f, g, h)  z(a(a(a(a(a(0, d), e), f), g), h))

int main()
{
  printf("%.2x\n", f(0x40, 0, 0, 0, 0));
  printf("%.2x\n", f(0x48, 0, 0, 1, 0xAA));
  printf("%.2x\n", f(0x01, 0, 0, 1, 0xAA));
  printf("%.2x\n", f(0x41, 0, 0, 0, 0));
  printf("%.2x\n", f(0x40 | 17, 0, 0, 0, 0));
  printf("%.2x\n", f(0x40 | 16, 0, 0, 0, 80));
  return 0;
}
