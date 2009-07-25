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
uint8_t i;

#ifndef LOGTEST_UNLIMITED
uint8_t j;
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

void send_debug_state()
{
  send_char_hd(log_state);
  send_char_hd(log_substate);
  send_char_hd(log_mode);
}

/* Our replacement ISR */
ISR(SPI_STC_vect)
{
  /* Debug the log_state before running the function */
  send_debug_state();

  /* Grab the received char and send it via the debug UART */
  hooked_SPDR = SPDR;

  send_char_hd(hooked_SPDR);
  send_char(' ');

  if (log_state == log_state_deselect_idle ||
      log_state == log_state_deselect)
  {
    /* The ISR will not set SPDR - it's finished */
    hooked_function();

    #ifndef LOGTEST_UNLIMITED
    if (j < 5)
    {
      /* So reset and restart */
      log_start();
      i = 0;
      j++;
    }
    #else
    log_start();
    i = 0;
    #endif
  }
  else
  {
    /* The ISR will continue looping */
    hooked_function();
    SPDR = hooked_SPDR;
  }

  /* Debug dump the transmitted character */
  send_char_hd(hooked_SPDR);
  send_char(' ');

  /* And the new log_state */
  send_debug_state();
  send_char('\n');
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
