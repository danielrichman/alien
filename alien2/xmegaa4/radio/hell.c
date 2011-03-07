/*
    Original and table source Copyright (C) 2010  James Coxon
    Rewrite Copyright (C) 2010  Daniel Richman

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
#include "hell.h"

static void hell_init();
static uint8_t hell_interrupt();
static PGM_P hell_getname(uint8_t t, uint8_t options);
static uint8_t helltab_get_data(uint8_t c, uint8_t n);

const struct radio_mode hell = { hell_init, hell_interrupt, hell_getname };

#define HELL_FREQ  2100
#define HELL_LINES 7
#define HELL_BITS  7
static uint8_t current_bit, current_line, current_line_num;

static void hell_init()
{
    radio_hw_timer_set(RADIO_HW_TIMER_DIV2, 32700);
    radio_hw_mode(RADIO_HW_MODE_TXOFF);

    radio_data_update();
    current_line = helltab_get_data(radio_data_current_byte, 0);

    current_bit = 0;
    current_line_num = 0;
}

static uint8_t hell_interrupt()
{
    if (current_line & (0x80 >> current_bit))
    {
        radio_hw_dac_set(HELL_FREQ);
        radio_hw_mode(RADIO_HW_MODE_TX);
    }
    else
    {
        radio_hw_mode(RADIO_HW_MODE_TXOFF);
    }

    current_bit++;

    if (current_bit >= HELL_BITS)
    {
        current_bit = 0;
        current_line_num++;

        if (current_line_num >= HELL_LINES)
        {
            uint8_t source_status;

            current_line_num = 0;
            source_status = radio_data_update();

            if (source_status == DATA_SOURCE_OK)
            {
                current_line = helltab_get_data(radio_data_current_byte, 0);
            }

            return source_status;
        }
        else
        {
            current_line = helltab_get_data(radio_data_current_byte,
                                            current_line_num);
        }
    }

    return RADIO_INTERRUPT_OK;
}

static char hell_short_name[] PROGMEM = "HELL";
static char hell_long_name[] PROGMEM = "Feldhellschreiber";

static PGM_P hell_getname(uint8_t t, uint8_t options)
{
    if (t == RADIO_NAME_SHORT)
    {
        return hell_short_name;
    }
    else
    {
        return hell_long_name;
    }
}

/*
 * The alphabet below has been arranged such that it consumes much less ROM.
 * You can see how it looked before and inspect the code that generated it in
 * /misc-c/pc/tables/hell.c
 *
 * For example,
 *   {'A', { 0b01111000, 0b00101100, 0b00100100, 0b00101100, 0b01111000 } },
 *
 * Makes the letter A:
 * Byte 12345
 * Bit
 * 0x01 (unused)
 * 0x02
 * 0x04  111 
 * 0x08 11 11
 * 0x10 1   1
 * 0x20 11111
 * 0x40 1   1
 * 0x80
 *
 * (Providing you look at it the right way up)
 * Note that bytes 0 and 6 are blank (0x00) and not stored in the table.
 *
 * That becomes 5byte item 1 in helltab_letters - "\x78\x2c\x24\x2c\x78"
 * (helltab_letters is 5 * 26 bytes long; 'A' to 'Z'.
 *  helltab_symbols is ',' to '9')
 */

static uint8_t helltab_letters[] PROGMEM =
    "\x78\x2c\x24\x2c\x78\x44\x7c\x54\x54\x28\x38\x6c\x44\x44\x28\x44"
    "\x7c\x44\x44\x38\x7c\x54\x54\x44\x44\x7c\x14\x14\x04\x04\x38\x6c"
    "\x44\x54\x34\x7c\x10\x10\x10\x7c\x00\x44\x7c\x44\x00\x60\x40\x40"
    "\x40\x7c\x7c\x10\x38\x28\x44\x7c\x40\x40\x40\x40\x7c\x08\x10\x08"
    "\x7c\x7c\x04\x08\x10\x7c\x38\x44\x44\x44\x38\x44\x7c\x54\x14\x1c"
    "\x38\x44\x64\xc4\xb8\x7c\x14\x14\x34\x58\x58\x54\x54\x54\x24\x04"
    "\x04\x7c\x04\x04\x7c\x40\x40\x40\x7c\x7c\x20\x10\x08\x04\x7c\x60"
    "\x7c\x40\x7c\x44\x28\x10\x28\x44\x04\x08\x70\x08\x04\x44\x64\x54"
    "\x4c\x64";

static uint8_t helltab_symbols[] PROGMEM =
    "\x80\xa0\x60\x00\x00\x00\x10\x10\x10\x00\x40\x40\x00\x00\x00\x40"
    "\x20\x10\x08\x04\x38\x64\x54\x4c\x38\x04\x04\x7c\x00\x00\x48\x64"
    "\x54\x4c\x40\x44\x44\x54\x54\x3c\x1c\x10\x10\x7c\x10\x40\x5c\x54"
    "\x54\x34\x3c\x52\x4a\x48\x30\x44\x24\x14\x0c\x04\x6c\x5a\x54\x5a"
    "\x6c\x08\x4a\x4a\x2a\x38";

static uint8_t helltab_get_data(uint8_t c, uint8_t n)
{
    const prog_uchar *location;

    if (n < 1 || n > 5)
    {
        return 0x00;
    }

    n--;

    if (c >= 'a' && c <= 'z')
    {
        location = helltab_letters + ((c - 'a') * 5);
    }
    else if (c >= 'A' && c <= 'Z')
    {
        location = helltab_letters + ((c - 'A') * 5);
    }
    else if (c >= ',' && c <= '9')
    {
        location = helltab_symbols + ((c - ',') * 5);
    }
    else
    {
        return 0x00;
    }

    return pgm_read_byte(location + n);
}
