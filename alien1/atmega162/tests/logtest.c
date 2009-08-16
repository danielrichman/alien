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
uint8_t msg[] = "Hello World "
                "abcdefghijklmnopqrstuvwxyz "
                "ABCDEFGHIJKLMNOPQRSTUV "
                "012456789 "
                "\n";
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
    send_char_hd(ba(log_timeout)[1]);
    send_char_hd(ba(log_timeout)[0]);
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

  /* We do not start the loop by calling log_tick/log_start, for accessing
   * variables in both ISRs and the main routine creates bad problems
   * (solved by declaring those variables volatile). I don't want to do that,
   * so will start it this way */
  SPDR = 0xFF;

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

  #ifndef LOGTEST_DEBUG
    /* Backup state incase of error, so we know where it failed */
    t = log_state;
    b = hooked_SPDR;
  #endif

  hooked_function();

  if (log_state == log_state_datawait ||
      log_state == log_state_deselect)
  {
    /* Due to whatever reason, The ISR will not set SPDR - it's finished.
     * So reset and restart */
    #ifdef LOGTEST_DEBUG
      if (log_state == log_state_datawait)
      {
        /* End of message. Reset message pointer, increase j */
        i = 0;
      }

      /* Only continue if we've looped less than 30 times */
      j++;

      if (j < 30)
      {
        log_start();
        SPDR = hooked_SPDR;
      }
    #else
      if (j == 1)
      {
        /* Flag the sending of a superblock */
        send_char('\n');
        j = 0;
      }

      /* Why has the loop ended? Find out and report it */
      if (log_state == log_state_datawait)
      {
        /* End of message. Reset message pointer */
        send_char('d');
        i = 0;
      }
      else
      {
        /* Failure. What error occured? */
        send_char_hd(t);
        send_char_hd(b);
        send_char(' ');
      }

      /* Ensure that the loop continues */
      log_start();
      SPDR = hooked_SPDR;
    #endif
  }
  else
  {
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

  c = msg[i];
  i++;

  return c;
}

/* To keep log.c's bit setting and clearing in system state happy */
payload_message latest_data;

