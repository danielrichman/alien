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

/* TODO: Some comments in the way of explaining what is going on here */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

int main(int argc, char **argv)
{
  uint8_t chksum, chksum_verify, chksum_verify_status, chksum_verify_reverse, chksum_printed;
  uint8_t option_newl, option_chkrm, option_help, options_valid;
  size_t i;
  uint8_t j, k, l;
  int m;

  option_newl   = 1;
  option_help   = 0;
  option_chkrm  = 1;
  options_valid = 1;

  if (argc == 2 && argv[1][0] == '-' && argv[1][1] != '\0')
  {
    j = strlen(argv[1]);

    for (i = 1; i < j; i++)
    {
      if (argv[1][i] == 'n' && option_newl)
      {
        option_newl  = 0;
      }
      else if (argv[1][i] == 'h' && !option_help)
      {
        option_help  = 1;
      }
      else if (argv[1][i] == 'c' && option_chkrm)
      {
        option_chkrm = 0;
      }
      else
      {
        options_valid = 0;
        break;
      }
    }
  }
  else if (argc != 1)
  {
    options_valid = 0;
  }

  if (option_help && (!option_newl || option_chkrm))
  {
    options_valid = 0;
  }

  if (!options_valid)
  {
    fprintf(stderr, "Usage: %s [-cnh]\n", argv[0]);
    fprintf(stderr, "Use -h for help\n");
    return 1;
  }

  if (option_help)
  {
    fprintf(stderr, "Usage: %s [-cnh]\n", argv[0]);
    fprintf(stderr, "     -h  displays this help\n");
    fprintf(stderr, "     -n  checksum the whole file, instead of each line\n");
    fprintf(stderr, "     -c  disables detection and validation of checksum\n");
    return 1;
  }

  chksum = 0;
  chksum_verify = 0;
  chksum_verify_status = 0;
  chksum_verify_reverse = 0;
  chksum_printed = 0;

  while (!feof(stdin))
  {
    m = getchar();

    if (m == EOF)
    {
      break;
    }

    k = ((uint8_t) m);

    if ((k < ' ' || k > '~') && k != '\n')
    {
      continue;
    }

    chksum_printed = 0;

    if (k == '\n')
    {
      if (option_newl)
      {
        if (option_chkrm)
        {
          if (chksum_verify_status == 3)
          {
            chksum ^= '*';
            chksum ^= chksum_verify_reverse;

            if (chksum == chksum_verify)
            {
              printf("*%.2X *%.2X OK\n",  chksum, chksum_verify);
            }
            else
            {
              printf("*%.2X *%.2X BAD\n", chksum, chksum_verify);
            }
          }
          else
          {
            printf("*%.2X MISSING\n", chksum);
          }
        }
        else
        {
          printf("*%.2X\n", chksum);
        }

        chksum_printed = 1;
        chksum = 0;
        chksum_verify_status = 0;
      }
    }
    else
    {
      if (option_chkrm)
      {
        if (k == '*')
        {
          chksum_verify_status = 1;
        }
        else if (chksum_verify_status == 1 || chksum_verify_status == 2)
        {
          if (k >= '0' && k <= '9')
          {
            l = k - '0';
          }
          else if (k >= 'a' && k <= 'f')
          {
            l = (k - 'a') + 10;
          }
          else if (k >= 'A' && k <= 'F')
          {
            l = (k - 'A') + 10;
          }
          else 
          {
            l = 255;
          }

          if (l != 255)
          {
            if (chksum_verify_status == 1)
            {
              chksum_verify_reverse = k;
              chksum_verify = l << 4;
              chksum_verify_status = 2;
            }
            else
            {
              chksum_verify_reverse ^= k;
              chksum_verify |= l;
              chksum_verify_status = 3;
            }
          }
          else
          {
            chksum_verify_status = 0;
          }
        }
        else if (chksum_verify_status == 3)
        {
          chksum_verify_status = 0;
        }
      }

      chksum ^= k;
    }
  }

  if (!chksum_printed)
  {
    if (option_chkrm)
    {
      if (chksum_verify_status == 3)
      {
        chksum ^= '*';
        chksum ^= chksum_verify_reverse;

        if (chksum == chksum_verify)
        {
          printf("*%.2X *%.2X OK\n",  chksum, chksum_verify);
        }
        else
        {
          printf("*%.2X *%.2X BAD\n", chksum, chksum_verify);
        }
      }
      else
      {
        printf("*%.2X MISSING\n", chksum);
      }
    }
    else
    {
      printf("*%.2X\n", chksum);
    }
  }

  return 0;
}

