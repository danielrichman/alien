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

#include "gps.h"  
#include "hexdump.h"
#include "messages.h"  
#include "radio.h" 
#include "sms.h"
#include "temperature.h"  
#include "timer1.h"

/* The radio will be on gpio 4 (high => mark:1.2V, low => space:1.0V). 
 * Potential Dividors with potentiometers will generate the 
 * required voltages
 * Arduino gpio4 = Port D; PD4 */

#define RADIO_MARK  PORTD |=   _BV(PD4)  /*   Set the bit; pb4 on  => mark  */
#define RADIO_SPACE PORTD &= ~(_BV(PD4)) /* Clear the bit; pb4 off => space */

/* radio_state #defines are in radio.h */

/* Global Variables */
uint8_t radio_state, radio_char;

/* 50hz timer interrupt */
void radio_proc()
{
  if (radio_state == radio_state_not_txing)
  {
    RADIO_MARK;
  }
  else if (radio_state == radio_state_stop_bit)
  {
    RADIO_MARK;

    radio_char = messages_get_char(&radio_data, message_type_radio);
    if (radio_char != 0)
    {
      radio_state = radio_state_start_bit;
    }
    else
    {
      radio_state = radio_state_not_txing;
    }
  }
  else if (radio_state == radio_state_start_bit)
  {
    RADIO_SPACE;
    radio_state = 0;    /* Get ready for the first bit! */
  }
  else
  {
    if (radio_char & _BV(radio_state))  /* If the bit is set... */
    {
      RADIO_MARK;
    }
    else
    {
      RADIO_SPACE;
    }

    radio_state++;                      /* Next bit */

    if (radio_state == radio_no_of_bits)
    {
      radio_state = radio_state_stop_bit;
    }
  }
}

void radio_init()
{
       /* Setup Radio State */
  radio_state = radio_state_not_txing;

       /* Setup Radio Outputs */
  DDRD  |= _BV(DDD4);     /* Set portD, pin4 as an output.   */
  RADIO_MARK;             /* Idle state = mark               */
}

/* This function is called after placing data in radio_data,
 * and signals to the radio that it should start TXing       */
void radio_send()
{
  radio_char  = messages_get_char(&radio_data, message_type_radio);
  radio_state = radio_state_start_bit;
}

