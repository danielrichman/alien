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

/* Deal with phone number hardcoding privacy */
#include "phone_no_private.h"

#ifndef ALIEN_PHONE_NO_PRIV_HEADER
  #error The phone number that the computer transmits to 
  #error is hardcoded in, however, in order to protect the 
  #error security of our phone number the hardcode is not 
  #error uploaded to svn. In order to specify a phone number, 
  #error copy phone_no_example.h to phone_no_private.h and 
  #error follow the instructions in there.
#endif

/* This macro takes the second phone no digit in the pair, then pushes it to
 * the most significant bits of the octet (ie, placing it first in the output,
 * thus swapping the pairs */
#define ph(i)  ((ALIEN_PHONE_NO_ ## i ## 2) << 4) + (ALIEN_PHONE_NO_ ## i ## 1)

/* Take the phone number digits and swap the pairs: */
uint8_t phone_number[12] = { ph(1), ph(2), ph(3), ph(4), ph(5), ph(6) };

uint8_t  sms_state, sms_substate;
uint16_t sms_temp;             /* Used when constructing the message octets */

ISR (USART_UDRE_vect)
{
  /* TODO UDRIE interrupt; sms.c */
}

/* It's not an _init like the other ones, because it's only called when sms.c
 * is needed, as it competes with gps.c for use of the UART. */
void sms_setup()
{
  /* gps.c will have set up for 8bit chars */

  /* Prepare to send... */
  

  /* UBRR = F_CPU/(16 * baudrate) - 1 
   *      = 16000000/16b - 1
   *      = 1000000/b - 1
   *      = 1000000/9600 - 1 = 103.16667 */
  UBRR0 = 103;

  /* Enable Transmit Mode and UDR empty interrupts */
  UCSR0B = ((_BV(TXEN0)) | (_BV(UDRIE0)));
}


