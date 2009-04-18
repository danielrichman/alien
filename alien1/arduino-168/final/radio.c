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

/* The radio will be on gpio 4 (high => mark:1.2V, low => space:1.0V). 
 * Potential Dividors with potentiometers will generate the 
 * required voltages
 * Arduino gpio4 = Port D; PD4 */

#define RADIO_MARK  PORTD |=   _BV(PD4)  /*   Set the bit; pb4 on  => mark  */
#define RADIO_SPACE PORTD &= ~(_BV(PD4)) /* Clear the bit; pb4 off => space */

#define radio_state_not_txing   0xFF
#define radio_state_start_bit   0xDD
#define radio_state_stop_bit    0xEE
#define radio_no_of_bits          7      /* 7bit ASCII */
/* 0x00 through 0x07 for radio_state represents bits */
/* Transmitting the stop bit sets the idle state too */

uint8_t radio_state = radio_state_not_txing;
uint8_t radio_char;

ISR (TIMER1_COMPA_vect)
{
  if (radio_state == radio_state_not_txing)
  {
    RADIO_MARK;
  }
  else if (radio_state == radio_state_stop_bit)
  {
    RADIO_MARK;

    /* TODO We need a new character */
    /* if (we_can_has_another_char)
     * {
     *   radio_char  = lots_of_chars[which_one];  
     *   radio_state = radio_state_start_bit;
     * }
     * else
     * {                                          */
         radio_state = radio_state_not_txing;
    /* }                                          */
  }
  else if (radio_state = radio_state_start_bit)
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

void radio_initialize()
{
       /* Setup Radio Outputs */
  DDRD  |= _BV(DDD4);     /* Set portD, pin4 as an output.  */
  RADIO_MARK;             /* Idle state = mark              */

       /* Setup timer1 frequency as 50Hz */
  /* Prescaler to FCPU/256; Clear timer on compare match *
   * Timer freq is 16000000/256 = 62500Hz
   * We want 50Hz; 62500/50 = 1250. So we want an interrupt every
   * 1250 timer1 ticks. */
  TCCR1B |= (_BV(CS02) | _BV(WGM12));
  OCR1A   = 1250;

       /* Enable Timer1 */
  TCNT1   = 0;            /* Reset timer */
  TIMSK1 |= _BV(OCIE1A);  /* Enable Compare Match Interrupts */
  sei();                  /* Turn on interrupts */
}

