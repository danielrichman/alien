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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
  unsigned char chksum;
  int i, len;

  if (argc != 2)
  {
    printf("Usage: %s <string to generate checksum from>\n", argv[0]);
    exit(-1);
  }

  len = strlen(argv[1]);
  chksum = 0;

  for (i = 0; i < len; i++)
  {
    chksum ^= argv[1][i];    
  }

  printf("*%.2X\n", chksum);

  return 0;
}

