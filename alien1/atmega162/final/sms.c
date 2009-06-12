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
#include "log.h"
#include "main.h"
#include "messages.h"
#include "radio.h"
#include "sms.h"
#include "statusled.h"
#include "temperature.h"
#include "timer1.h"
#include "timer3.h"
#include "watchdog.h"

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

/* Static data - always the same for every sms sent.
 * See trunk/misc-c/sms-example-v2.c                  */

/* NOTE: We hardcode the maximum length that it could be, then if it falls
 * short we pad with spaces. */
/* Calculate the octet count from the max length. The huge #define should be
 * evaluated before compilation by the awesomness of gcc */
#define sms_no_of_msg_bits          (messages_max_length * 7)
#define sms_no_of_part_octet_bits   (sms_no_of_msg_bits % 8)
#define sms_no_of_msg_bits_8        (sms_no_of_part_octet_bits == 0 ?        \
                                        sms_no_of_msg_bits :                 \
                                        sms_no_of_msg_bits +                 \
                                           (8 - sms_no_of_part_octet_bits))
#define sms_no_of_msg_octets        (sms_no_of_msg_bits_8 / 8)
#define sms_no_of_octets            (sms_no_of_msg_octets + 14)
#define sms_no_of_octets_d1         ('0' + (sms_no_of_octets / 10))
#define sms_no_of_octets_d2         ('0' + (sms_no_of_octets % 10))

uint8_t sms_formatcmd[] = { 'A',  'T',  '+',  'C',  'M',  'G', 'F',  '=',  '0',
                            '\r', '\n' };
uint8_t sms_cmdstart[]  = { 'A',  'T',  '+',  'C',  'M',  'G', 'S',  '=', 
                            sms_no_of_octets_d1,  sms_no_of_octets_d2, 
                            '\r', '\n' };

/* This is the first bit of the hexstring, before the message data. 
 * 0011000C91xxxxxxxxxxxx0000AAyy  where xx...xx is the phone number,
 * and yy is the hardcoded length (see NOTE above)
 * See sms-example-v2.c
 * Because this is hexdumped, we represents as bytes to save space */
uint8_t sms_hexstart[] = { 0x00, 0x11, 0x00, 0x0C, 0x91, 
                           ph(1), ph(2), ph(3), ph(4), ph(5), ph(6),
                           0x00, 0x00, 0xAA, messages_max_length };

uint8_t sms_cmdend[]   = { 0x1a, '\r', '\n' };

uint8_t  sms_state, sms_mode;
uint8_t  sms_substate, sms_tempbits;
uint16_t sms_temp;             /* Used when constructing the message octets */

#define SMS_ENABLE_ISR   UCSR1B |=   _BV(UDRIE1);
#define SMS_DISABLE_ISR  UCSR1B &= ~(_BV(UDRIE1));

ISR (USART1_UDRE_vect)
{
  uint16_t c;

  switch (sms_state)
  {
    case sms_state_formatcmd:
      UDR1 = sms_formatcmd[sms_substate];
      sms_substate++;

      if (sms_substate == sizeof(sms_formatcmd))
      {
        sms_substate = 0;
        sms_state++;
	sms_wait();
      }
      break;

    case sms_state_cmdstart:
      UDR1 = sms_cmdstart[sms_substate];
      sms_substate++;

      if (sms_substate == sizeof(sms_cmdstart))
      {
        sms_substate = 0;
        sms_state++;
        sms_wait();
      }
      break;

    case sms_state_hexstart_a:
      UDR1 = hexdump_a(sms_hexstart[sms_substate]);
      sms_state++;
      break;

    case sms_state_hexstart_b:
      UDR1 = hexdump_b(sms_hexstart[sms_substate]);
      sms_substate++;

      if (sms_substate == sizeof(sms_hexstart))
      {
        sms_state++;
        sms_substate = 0;
        sms_temp = 0;
        sms_tempbits = 0;
      }
      else
      {
        sms_state--;
      }
      break;

    case sms_state_messagehex_a:
      /* Fill up the current byte with data - add bit-chunks of 7 until
       * we have more than eight. Also, don't go over the max length */
      while (sms_tempbits < 8 && sms_substate != messages_max_length)
      {
        c = messages_get_char(&sms_data);

        if (c == 0) 
        {
          c = ' ';   /* If it's ended, just add spaces */
        }

        /* sms_tempbits represents how many bits we've already written to
         * the first byte of sms_tempbits. We shift left to align c to the
         * empty space. We can shift left far enough as it's been cast to 
         * uint16_t */
        sms_temp |= c << sms_tempbits;

        sms_tempbits += 7;
	sms_substate++;
      }

      /* Start sending the first byte of sms_temp */
      UDR1 = hexdump_a( sms_temp );
      sms_state++;

      break;

    case sms_state_messagehex_b:
      UDR1 = hexdump_b( sms_temp );

      /* Get rid of the byte we have just sent, keep MSB as there
       * may be bits set in it */
      sms_tempbits -= 8;
      sms_temp = (sms_temp >> 8) & 0xFF;

      /* If there's more to send 
       * (ie. there's chars left or the buffer isn't empty) */
      if (sms_substate != messages_max_length || sms_temp != 0)
      {
        sms_state--;
      }
      else
      {
        sms_state++;
        sms_substate = 0;
      }

      break;

    case sms_state_cmdend:
      UDR1 = sms_cmdend[sms_substate];
      sms_substate++;

      if (sms_substate == sizeof(sms_cmdend))
      {
        sms_substate = 0;
        sms_state = sms_state_formatcmd;
        sms_mode  = sms_mode_null;
        SMS_DISABLE_ISR;
      }
      break;
  }
}

void sms_wait()
{
  SMS_DISABLE_ISR;
  sms_mode = sms_mode_waiting;

  timer3_clear();
  timer3_start();
}

void sms_start()
{
  /* This gets called when we want to start sending an sms and 
   * after each wait finishes */

  sms_mode = sms_mode_busy;
  SMS_ENABLE_ISR;
}  

void sms_init()
{
  /* UBRR = F_CPU/(16 * baudrate) - 1 
   *      = 16000000/16b - 1
   *      = 1000000/b - 1
   *      = 1000000/9600 - 1 = 103.16667 */
  UBRR1L = 103;

  /* Enable Transmit Mode only */
  UCSR1B = _BV(TXEN1);
}


