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

#include "radio.h"
#include "messages.h"

/* The radio will be on gpio 4 (high => mark:1.2V, low => space:1.0V). 
 * Potential Dividors with potentiometers will generate the 
 * required voltages
 * Arduino gpio4 = Port D; PD4 */

#define RADIO_MARK  PORTD |=   _BV(PD4)  /*   Set the bit; pb4 on  => mark  */
#define RADIO_SPACE PORTD &= ~(_BV(PD4)) /* Clear the bit; pb4 off => space */

/* radio_state #defines are in radio.h */

/* Enable Timer at FCPU/256  -  Set bit CS02 in TCCR1B          */
#define  ENABLE_TIMER1_50HZ   TCCR1B |=   _BV(CS02);
/* Disable Timer; set prescale to "No Clock"  -  Clear bit CS02 */
#define DISABLE_TIMER1_50HZ   TCCR1B &= ~(_BV(CS02));

/* Global Variables */
uint8_t radio_state, radio_char;

/* 50hz timer interrupt */
ISR (TIMER1_COMPA_vect)
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
      DISABLE_TIMER1_50HZ;
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

       /* Setup timer1 compare-match settings */
  /* TCCR1B: Clear timer on compare match    (Set bit WGM12) */
  TCCR1B |= _BV(WGM12);
  /* TIMSK1: Enable Compare Match Interrupts (Set bit OCIE1A)*/
  TIMSK1 |= _BV(OCIE1A);

  /* For now, the timer is disabled. We will turn it on      *
   * when we need it (See ENABLE_TIMER1_50HZ macro)          */
  DISABLE_TIMER1_50HZ;

       /* Setup timer1 frequency as 50Hz */
  /* Prescaler will beFCPU/256 (Set bit CS02). 
   * So Timer freq will be 16000000/256 = 62500Hz
   * We want 50Hz; 62500/50 = 1250. So we want an 
   * interrupt every 1250 timer1 ticks. */
  OCR1A   = 1250;
}

/* This function is called after placing data in radio_data,
 * and signals to the radio that it should start TXing       */
void radio_send()
{
  if (radio_state != radio_state_not_txing)
  {
    /* The radio is busy */
    return;
  }

  radio_char  = messages_get_char(&radio_data, message_type_radio);
  radio_state = radio_state_start_bit;

       /* Enable Timer1 */
  TCNT1   = 0;            /* Reset timer */
  ENABLE_TIMER1_50HZ;     /* Go go go!   */
}

