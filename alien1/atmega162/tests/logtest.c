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

/* This test program features a lot of squashing, hacks and macros to bend
 * log.c into the shape we want it. A lot of things are used before they are
 * squashed, forex. log_start is used in main() with SPDR as its normal self
 * before being squashed to hooked_SPDR for the ISR. Watch out */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "../final/log.h"
void hooked_function();
uint8_t hooked_SPDR;

/* extern-expose some more variables */
extern uint8_t  log_substate, log_mode;
extern uint8_t  log_command[6];
extern uint16_t log_timeout;
extern uint32_t log_position, log_position_b;

/* From Radio Test */
uint8_t msg[] = {'H', 'e', 'l', 'l', 'o',  ' ', 'W', 'o', 'r', 'l', 'd', '\n',
                 'a', 'b', 'c', 'd', 'e',  'f', 'g', 'h', 'i', 'j', 'k', 'l', 
                 'm', 'n', 'o', 'p', 'q',  'r', 's', 't', 'u', 'v', 'w', 'x', 
                 'y', 'z', 'A', 'B', 'C',  'D', 'E', 'F', 'G', 'H', 'I', 'J', 
                 'K', 'L', 'M', 'N', 'O',  'P', 'Q', 'R', 'S', 'T', 'U', 'V', 
                 'W', 'X', 'Y', 'Z', '\n', '0', '1', '2', '3', '4', '5', '6', 
                 '7', '8', '9', '\n' };
uint8_t i, j;

#ifndef LOGTEST_DEBUG
  uint8_t t, b;
#endif

/* Some USART Functions */
void send_char(uint8_t c)
{
  loop_until_bit_is_set(UCSR0A, UDRE0);
  UDR0 = c;
}

#include "../final/hexdump.h"

void send_char_hd(uint8_t c)
{
  send_char(hexdump_a(c));
  send_char(hexdump_b(c));
}

#ifdef LOGTEST_DEBUG
  void send_debug_state()
  {
    send_char_hd(log_state);
    send_char_hd(log_substate);
    send_char_hd(log_mode);
    send_char('-');
    send_char_hd(j);
    send_char_hd(i);
  }
#endif

int main(void)
{
  /* Setup logging */
  log_init();

  /* Setup UART */
  UCSR0B = ((_BV(TXEN0)));
  UBRR0L = 8;

  /* Start! */
  sei();
  log_start();
  for (;;)    sleep_mode();
}

/* Our replacement ISR */
ISR(SPI_STC_vect)
{
  #ifdef LOGTEST_DEBUG
    /* Debug the log_state before running the function */
    send_debug_state();
    send_char(' ');
  #endif

  /* Grab the received char and send it via the debug UART */
  hooked_SPDR = SPDR;

  #ifdef LOGTEST_DEBUG
    send_char_hd(hooked_SPDR);
    send_char(' ');
  #endif

  #ifndef LOGTEST_DEBUG
    /* Flag the fact that a superblock has been written */
    if (log_state == log_state_writing_super)
    {
      j = 1;
    }
  #endif

  if (log_state == log_state_deselect_idle ||
      log_state == log_state_deselect)
  {
    /* The ISR will not set SPDR - it's finished */
    hooked_function();

    /* So reset and restart */
    #ifdef LOGTEST_DEBUG
      if (j < 5)
      {
        log_start();

        i = 0;
        j++;
      }
    #else
      log_start();
      i = 0;

      if (j == 1)
      {
        /* Flag the sending of a superblock */
        send_char('\n');
        j = 0;
      }

      if (log_state != log_state_idle)
      {
        /* What error occured? */
        send_char_hd(t);
        send_char_hd(b);
        send_char(' ');
      }
      else
      {
        send_char('d');
      }
    #endif
  }
  else
  {
    #ifndef LOGTEST_DEBUG
      /* Backup state incase of error, so we know where it failed */
      t = log_state;
      b = hooked_SPDR;
    #endif

    /* The ISR will continue looping */
    hooked_function();

    /* Deal with the transmitted character */
    SPDR = hooked_SPDR;
  }

  #ifdef LOGTEST_DEBUG 
    /* Debug dump the transmitted character */
    send_char_hd(hooked_SPDR);
    send_char(' ');

    /* And the new log_state */
    send_debug_state();
    send_char('\n');
  #endif
}

/* Now squash log.c's ISR with some macros. */
#undef ISR
#define ISR(vname)   void hooked_function()

/* And replace SPDR with our new one */
#undef SPDR
#define SPDR hooked_SPDR

/* Avoid useless use of bss space... */
#define messages_get_char(an_unused_variable)  messages_get_char()

/* Now include log.c */
#include "../final/log.c"

/* Simple message generator */
uint8_t messages_get_char(payload_message *data)
{
  uint8_t c;

  if (i == sizeof(msg))   return 0;

  c = msg[i];
  i++;

  return c;
}

