/*
    Original source http://brainwagon.org http://brainwagon.org/2009/11/14/
        Simple Arduino Morse Beacon
        Written by Mark VandeWettering K6HX
        Email: k6hx@arrl.net

        This code is so trivial that I'm releasing it completely without
        restrictions.  If you find it useful, it would be nice if you dropped
        me an email, maybe plugged my blog @ http://brainwagon.org or included
        a brief acknowledgement in whatever derivative you create, but that's
        just a courtesy.  Feel free to do whatever.

    Some modifications by James Coxon
    Some modifications by Daniel Richman
*/

/*
 * This turns the table below into a more compact form.
 * See /alien2/xmegaa4/radio/morse.c
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

struct t_mtab { unsigned char c, pat; } ;

#define SPACE 0

struct t_mtab morsetab[] = {
        {',', 115},
        {'-', SPACE},
        {'.', 106},
        {'/', 41},
        {'0', 63},
        {'1', 62},
        {'2', 60},
        {'3', 56},
        {'4', 48},
        {'5', 32},
        {'6', 33},
        {'7', 35},
        {'8', 39},
        {'9', 47},
        {':', SPACE},
        {';', SPACE},
        {'<', SPACE},
        {'=', SPACE},
        {'>', SPACE},
        {'?', 76},
        {'@', SPACE},
        {'A', 6},
        {'B', 17},
        {'C', 21},
        {'D', 9},
        {'E', 2},
        {'F', 20},
        {'G', 11},
        {'H', 16},
        {'I', 4},
        {'J', 30},
        {'K', 13},
        {'L', 18},
        {'M', 7},
        {'N', 5},
        {'O', 15},
        {'P', 22},
        {'Q', 27},
        {'R', 10},
        {'S', 8},
        {'T', 3},
        {'U', 12},
        {'V', 24},
        {'W', 14},
        {'X', 25},
        {'Y', 29},
        {'Z', 19},
} ;

#define N_MORSE  (sizeof(morsetab)/sizeof(morsetab[0]))

int main(int argc, char **argv)
{
  int i, c;
  c = morsetab[0].c;
  for (i = 0; i < N_MORSE; i++)
  {
    if (c != morsetab[i].c)  exit(EXIT_FAILURE);
    c++;
    printf("\\x%02x", morsetab[i].pat);
  }
  printf("\n");
  return 0;
}
