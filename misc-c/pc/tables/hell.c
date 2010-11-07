/*
    Copyright (C) 2010  James Coxon
    Some modifications Copyright (C) 2010  Daniel Richman

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTA0bILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    For a full copy of the GNU General Public License, 
    see <http://www.gnu.org/licenses/>.
*/

/*
 * This turns the table below into a more compact form.
 * See /alien2/xmegaa4/radio/hell.c
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

struct t_htab { char c; int hellpat[5]; } ;

struct t_htab helltab_symbols[] = {
  {',', { 0b10000000, 0b10100000, 0b01100000, 0b00000000, 0b00000000 } },
  {'-', { 0b00000000, 0b00010000, 0b00010000, 0b00010000, 0b00000000 } },
  {'.', { 0b01000000, 0b01000000, 0b00000000, 0b00000000, 0b00000000 } },
  {'/', { 0b01000000, 0b00100000, 0b00010000, 0b00001000, 0b00000100 } },
  {'0', { 0b00111000, 0b01100100, 0b01010100, 0b01001100, 0b00111000 } },
  {'1', { 0b00000100, 0b00000100, 0b01111100, 0b00000000, 0b00000000 } },
  {'2', { 0b01001000, 0b01100100, 0b01010100, 0b01001100, 0b01000000 } },
  {'3', { 0b01000100, 0b01000100, 0b01010100, 0b01010100, 0b00111100 } },
  {'4', { 0b00011100, 0b00010000, 0b00010000, 0b01111100, 0b00010000 } },
  {'5', { 0b01000000, 0b01011100, 0b01010100, 0b01010100, 0b00110100 } },
  {'6', { 0b00111100, 0b01010010, 0b01001010, 0b01001000, 0b00110000 } },
  {'7', { 0b01000100, 0b00100100, 0b00010100, 0b00001100, 0b00000100 } },
  {'8', { 0b01101100, 0b01011010, 0b01010100, 0b01011010, 0b01101100 } },
  {'9', { 0b00001000, 0b01001010, 0b01001010, 0b00101010, 0b00111000 } }
};

struct t_htab helltab_letters[] = {
  {'A', { 0b01111000, 0b00101100, 0b00100100, 0b00101100, 0b01111000 } },
  {'B', { 0b01000100, 0b01111100, 0b01010100, 0b01010100, 0b00101000 } },
  {'C', { 0b00111000, 0b01101100, 0b01000100, 0b01000100, 0b00101000 } },
  {'D', { 0b01000100, 0b01111100, 0b01000100, 0b01000100, 0b00111000 } },
  {'E', { 0b01111100, 0b01010100, 0b01010100, 0b01000100, 0b01000100 } },
  {'F', { 0b01111100, 0b00010100, 0b00010100, 0b00000100, 0b00000100 } },
  {'G', { 0b00111000, 0b01101100, 0b01000100, 0b01010100, 0b00110100 } },
  {'H', { 0b01111100, 0b00010000, 0b00010000, 0b00010000, 0b01111100 } },
  {'I', { 0b00000000, 0b01000100, 0b01111100, 0b01000100, 0b00000000 } },
  {'J', { 0b01100000, 0b01000000, 0b01000000, 0b01000000, 0b01111100 } },
  {'K', { 0b01111100, 0b00010000, 0b00111000, 0b00101000, 0b01000100 } },
  {'L', { 0b01111100, 0b01000000, 0b01000000, 0b01000000, 0b01000000 } },
  {'M', { 0b01111100, 0b00001000, 0b00010000, 0b00001000, 0b01111100 } },
  {'N', { 0b01111100, 0b00000100, 0b00001000, 0b00010000, 0b01111100 } },
  {'O', { 0b00111000, 0b01000100, 0b01000100, 0b01000100, 0b00111000 } },
  {'P', { 0b01000100, 0b01111100, 0b01010100, 0b00010100, 0b00011100 } },
  {'Q', { 0b00111000, 0b01000100, 0b01100100, 0b11000100, 0b10111000 } },
  {'R', { 0b01111100, 0b00010100, 0b00010100, 0b00110100, 0b01011000 } },
  {'S', { 0b01011000, 0b01010100, 0b01010100, 0b01010100, 0b00100100 } },
  {'T', { 0b00000100, 0b00000100, 0b01111100, 0b00000100, 0b00000100 } },
  {'U', { 0b01111100, 0b01000000, 0b01000000, 0b01000000, 0b01111100 } },
  {'V', { 0b01111100, 0b00100000, 0b00010000, 0b00001000, 0b00000100 } },
  {'W', { 0b01111100, 0b01100000, 0b01111100, 0b01000000, 0b01111100 } },
  {'X', { 0b01000100, 0b00101000, 0b00010000, 0b00101000, 0b01000100 } },
  {'Y', { 0b00000100, 0b00001000, 0b01110000, 0b00001000, 0b00000100 } },
  {'Z', { 0b01000100, 0b01100100, 0b01010100, 0b01001100, 0b01100100 } }
};

#define SIZEIFY(s) (sizeof(s)/sizeof((s)[0]))
#define N_HELL_symbols SIZEIFY(helltab_symbols)
#define N_HELL_letters SIZEIFY(helltab_letters)

void do_it(char *name, struct t_htab array[], int len)
{
  int i, j, c;

  printf("%s ", name);

  c = array[0].c;
  for (i = 0; i < len; i++)
  {
    if (c != array[i].c)  exit(EXIT_FAILURE);
    c++;

    for (j = 0; j < 5; j++)
    {
      printf("\\x%02x", array[i].hellpat[j]);
    }
  }

  printf("\n");
}

#define do_it_good(n) do_it(#n, helltab_ ## n, N_HELL_ ## n)

int main(int argc, char **argv)
{
  do_it_good(symbols);
  do_it_good(letters);
  return 0;
}
