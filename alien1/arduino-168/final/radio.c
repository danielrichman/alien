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

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdint.h>
#include <stdlib.h>

#include "camera.h"
#include "gps.h"  
#include "hexdump.h"
#include "main.h"
#include "messages.h"  
#include "radio.h" 
#include "sms.h"
#include "temperature.h"  
#include "timer1.h"

/* The radio will be on gpio 4 and 3.
 * Arduino gpio4 = Port D, PD4; gpio3 = Port D, PD3 */

/* radio_state #defines are in radio.h */

#define radio_space             0
#define radio_mark              1

#define radio_no_of_bits        7      /* 7bit ASCII             */
#define radio_pause_length      25     /* at 50hz, half a second */

/* Global Variables */
uint8_t radio_state, radio_substate, radio_char;

/* 50hz timer interrupt */
void radio_proc()
{
  uint8_t rb;

  /* Default to a mark. We mark when idle */
  rb = radio_mark;

  switch (radio_state)
  {
    case radio_state_start_bit:
      rb = radio_space;
      radio_state++;         /* Move onto _data_bits */
      radio_substate = 0;    /* Get ready for the first bit! */
      break;

    case radio_state_data_bits:
      rb = radio_char & _BV(radio_substate);   /* Select the bit */
      radio_substate++;                        /* Next bit on next loop */

      if (radio_substate == radio_no_of_bits)
      {
        radio_state++;       /* Move onto stop bits */
      }
      break;

    case radio_state_stop_bit_a:
      /* Defaults to the required mark */
      radio_state++;
      break;

    case radio_state_stop_bit_b:
      /* Defaults to the required mark */

      /* Try to get a new char... */
      radio_char = messages_get_char(&radio_data);

      if (radio_char != 0)
      {
        /* If we have a new char, go back to the start bit and loop again */
        radio_state = radio_state_start_bit;
      }
      else
      {
        /* Otherwise, goto the pausing section */
        radio_state++;
        radio_substate = 0;
      }
      break;

    case radio_state_pause:
      rb = radio_mark;

      /* A brief pause at the end of each transmission. */
      if (radio_substate == radio_pause_length)
      {
        radio_state = radio_state_not_txing;
      }
      else
      {
        radio_substate++;
      }

      break;
  }

  if (rb)
  {
    /* Mark:  PD4 on PD3 off */
    PORTD &= ~(_BV(PD3));
    PORTD |=   _BV(PD4);
  }
  else 
  {
    /* Space: PD3 on PD4 off */
    PORTD &= ~(_BV(PD4));
    PORTD |=   _BV(PD3);
  }
}

void radio_init()
{
       /* Setup Radio Outputs */
  DDRD  |= _BV(DDD4);     /* Set portD, pin4 as an output.   */
  DDRD  |= _BV(DDD3);     /* Set portD, pin3 as an output.   */

       /* Idle state = mark   */
  PORTD &= ~(_BV(PD3));
  PORTD |=   _BV(PD4);
}

/* This function is called after placing data in radio_data,
 * and signals to the radio that it should start TXing       */
void radio_send()
{
  radio_char  = messages_get_char(&radio_data);
  radio_state = radio_state_start_bit;
}

