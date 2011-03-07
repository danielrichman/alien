/*
    The DomEX22 code below originates from the fl-digi program;
    it has since been modified by CUSF and then myself. Therefore,

    fldigi/src/dominoex/dominoex.cxx & fldigi/include/dominoex.h
       Copyright (C) 2001, 2002, 2003
                  Tomi Manninen (oh2bns@sral.fi)
       Copyright (C) 2006
                  Hamish Moffatt (hamish@debian.org)
       Copyright (C) 2006, 2008-2009
                  David Freese (w1hkj@w1hkj.com)

       based on code in gmfsk

    fldigi/src/dominoex/dominovar.cxx
       Copyright (C) 2001, 2002, 2003
                  Tomi Manninen (oh2bns@sral.fi)

    Copyright (C) 2010  CU Spaceflight
    Copyright (C) 2010  Daniel Richman

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

#include <stdint.h>
#include <avr/pgmspace.h>

#include "radio.h"
#include "hardware.h"
#include "domex.h"

static void domex_init();
static uint8_t domex_interrupt();
static PGM_P domex_getname(uint8_t t, uint8_t options);
static uint16_t domex_get_nibbles(uint8_t c);

const struct radio_mode domex = { domex_init, domex_interrupt, domex_getname };

static uint8_t current_tone;
static uint16_t current_nibbles;

static void domex_init()
{
    radio_hw_timer_set(RADIO_HW_TIMER_DIV8, 46500);
    radio_hw_mode(RADIO_HW_MODE_TX);

    /* don't reset current_tone */
    radio_data_update();
    current_nibbles = domex_get_nibbles(radio_data_current_byte);
}

#define NUM_TONES 18
#define BASE_VALUE 2103
#define TONE_SHIFT 36

static uint8_t domex_interrupt()
{
    uint8_t nibble;

    nibble = current_nibbles & 0xF;
    current_tone = (current_tone + 2 + nibble);

    if (current_tone >= NUM_TONES)
    {
        current_tone -= NUM_TONES;
    }

    radio_hw_dac_set(BASE_VALUE + (current_tone * TONE_SHIFT));

    /*
     * domex_get_nibbles returns a 16bit value, 0x0cba where a, b, and c are
     * the nibbles to be sent, in that order. So we send a nibble and slide
     * current_nibbles to the right.
     */
    current_nibbles >>= 4;

    /*
     * The first nibble will not have the MSB set, but any multi-nibble
     * chars will have 0x08 set in their "continuation nibbles"
     */
    if (current_nibbles & 0x08)
    {
        return RADIO_INTERRUPT_OK;
    }
    else
    {
        uint8_t source_status;
        source_status = radio_data_update();

        if (source_status == DATA_SOURCE_OK)
        {
            current_nibbles = domex_get_nibbles(radio_data_current_byte);
        }

        return source_status;
    }
}

static char domex_short_name[] PROGMEM = "DmX22";
static char domex_long_name[] PROGMEM = "DominoEX 22";

static PGM_P domex_getname(uint8_t t, uint8_t options)
{
    if (t == RADIO_NAME_SHORT)
    {
        return domex_short_name;
    }
    else
    {
        return domex_long_name;
    }
}

/*
 * The alphabet below has been arranged such that it consumes much less ROM.
 * You can see how it looked before and inspect the code that generated it in
 * /misc-c/pc/tables/domex.c
 *
 * e.g., The first two sets are { 1,15, 9}, { 1,15,10},
 * that is, 0x1, 0xf, 0x9, 0x1, 0xf, 0xa
 *
 * We represent each set as a 16 bit integer, eg. (0x09f1 for the first set)
 * so that when reading it we can use the bitmask 0xf to grab the first
 * number, shift by 4 to the right, and repeat. Because the AVR is little
 * endian the first set is therefore \xf1 \x09
 *
 * Since there are only three numbers in each symbol 2 symbols can actually
 * fit into 2 bytes. We take the first number of the second symbol and fill
 * the empty space in the first byte, to get, for example
 *   0   1   2
 *   \xf1\x19\xaf
 *
 * If we read a 16bit int at [0], then we get 0x19f1 (little endian), 
 * meaning that the following tones are transmitted: 1, 15, 9. The final
 * 1 (that belongs to the second set) is ignored since it doesn't have
 * the MSB set (ie & 0x8).
 *
 * If we read a 16bit int at [1], we get 0xaf19. We discard the 9 since
 * it belongs to [0], and then have our data 0x0af1 which we can use.
 * (see transmit_dominoex_character).
 */
static uint8_t varicode[] PROGMEM =
    "\xf1\x19\xaf\xf1\x1b\xcf\xf1\x1d\xef\xf1\x2f\x88\xc2\x20\x98\x82"
    "\x2a\xb8\x82\x2c\x0d\x82\x2d\xe8\x82\x2f\x89\x92\x29\xa9\x92\x2b"
    "\xc9\x92\x2d\xe9\x92\x2f\x8a\xa2\x29\xaa\xa2\x2b\xca\xa2\x2d\xea"
    "\x00\x70\x0b\x80\x0e\xba\x90\x0a\x99\x80\x7f\x0a\x80\x0c\xb8\x90"
    "\x0d\x88\xb2\x70\x0e\xd7\x00\x98\xf3\x40\x0a\xf4\x50\x09\x86\x50"
    "\x0c\xe5\x60\x0c\xb6\x60\x0e\x80\x0a\xd8\xa0\x78\x0f\x90\x7f\x0c"
    "\x90\x38\x09\xe4\x30\x0c\xe3\x30\x08\xc4\x50\x08\xa5\x30\x0a\x87"
    "\x60\x0a\xb4\x40\x08\xd4\x30\x0b\x94\x60\x0f\xd3\x20\x0f\xe2\x50"
    "\x0b\xd6\x50\x0d\xf5\x60\x09\x97\x00\xea\xa0\x09\xfa\xa0\x0a\xc9"
    "\x90\x4b\x00\xb1\x00\x0c\xb0\x10\x00\xf0\x10\x09\xa0\x50\x00\xa2"
    "\x10\x0e\x90\x00\x0e\x06\x30\x00\x81\x20\x08\x07\x00\x08\x02\x00"
    "\x0d\xd1\x10\x0c\xf1\x10\x0a\x92\x00\xca\x90\x0e\xda\xb0\x28\xfa"
    "\xb2\x28\x9b\xb2\x2a\xbb\xb2\x2c\xdb\xb2\x2e\xfb\xc2\x28\x9c\xc2"
    "\x2a\xbc\xc2\x2c\xdc\xc2\x2e\xfc\xd2\x28\x9d\xd2\x2a\xbd\xd2\x2c"
    "\xdd\xd2\x2e\xfd\xe2\x28\x9e\xe2\x2a\xbe\xe2\x2c\xde\xe2\x2e\xfe"
    "\xb0\x09\xab\xb0\x0b\xcb\xb0\x0d\xeb\xb0\x0f\x8c\xc0\x09\xac\xc0"
    "\x0b\xcc\xc0\x0d\xec\xc0\x0f\x8d\xd0\x09\xad\xd0\x0b\xcd\xd0\x0d"
    "\xed\xd0\x0f\x8e\xe0\x09\xae\xe0\x0b\xce\xe0\x0d\xee\xe0\x0f\x8f"
    "\xf0\x09\xaf\xf0\x0b\xcf\xf0\x0d\xef\xf0\x1f\x88\x81\x19\xa8\x81"
    "\x1b\xc8\x81\x1d\xe8\x81\x1f\x89\x91\x19\xa9\x91\x1b\xc9\x91\x1d"
    "\xe9\x91\x1f\x8a\xa1\x19\xaa\xa1\x1b\xca\xa1\x1d\xea\xa1\x1f\x8b"
    "\xb1\x19\xab\xb1\x1b\xcb\xb1\x1d\xeb\xb1\x1f\x8c\xc1\x19\xac\xc1"
    "\x1b\xcc\xc1\x1d\xec\xc1\x1f\x8d\xd1\x19\xad\xd1\x1b\xcd\xd1\x1d"
    "\xed\xd1\x1f\x8e\xe1\x19\xae\xe1\x1b\xce\xe1\x1d\xee\xe1\x1f\x8f"
    "\xf6";

static uint16_t domex_get_nibbles(uint8_t c)
{
    uint16_t location, data;

    /* 2 nibbles packed in every 3 bytes; multiply by (3/2) */
    location = ((c * 3) / 2);
    data = pgm_read_word(&(varicode[location]));

    /* If it was odd, discard the lowest 4 bits */
    if (c & 0x01)
    {
        data >>= 4;
    }

    return data;
}
