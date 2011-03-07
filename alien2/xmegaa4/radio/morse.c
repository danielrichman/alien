/*
    Original source http://brainwagon.org http://brainwagon.org/2009/11/14/
        Simple Arduino Morse Beacon
        Written by Mark VandeWettering K6HX
        Email: k6hx@arrl.net

        This code is so trivial that I'm releasing it completely without
        restrictions.  If you find it useful, it would be nice if you dropped
        me an email, maybe plugged my blog @ http://brainwagon.org or included
        a brief acknowledgement in whatever derivative you create, but that's
        just a courtesy.  Feel free to do whatever.

    Some modifications Copyright (C) 2010 James Coxon
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
#include "morse.h"

static void morse_init();
static uint8_t morse_interrupt();
static PGM_P morse_getname(uint8_t t, uint8_t options);
static uint8_t morse_get_data(uint8_t c);

const struct radio_mode morse = { morse_init, morse_interrupt, morse_getname };

static uint8_t current_data, current_state;

#define MORSE_FREQ   2100

#define STATE_START  0
#define STATE_STOP   10
#define STATE_NSTOP  20 /* get a new byte, then continue */
/*
 * state is incremented 1 every interrupt. If state is set to STATE_DASH,
 * it will interrupt 3 times before reaching STATE_STOP
 */
#define STATE_DIT    (STATE_STOP - 1)
#define STATE_DASH   (STATE_STOP - 3)
#define STATE_SPACE  (STATE_NSTOP - 5) /* plus two gaps makes 7 */
#define STATE_CHRGAP (STATE_NSTOP - 1) /* plus two gaps makes 3 */

static void morse_init()
{
    /* approx 26WPM */
    radio_hw_timer_set(RADIO_HW_TIMER_DIV64, 5000);
    radio_hw_mode(RADIO_HW_MODE_TXOFF);

    radio_data_update();
    current_data = morse_get_data(radio_data_current_byte);

    current_state = STATE_START;
}

static uint8_t morse_interrupt()
{
    uint8_t source_status;

    switch (current_state)
    {
        case STATE_START:
            if (current_data == 0x00)
            {
                current_state = STATE_SPACE;
            }
            else if (current_data == 0x01)
            {
                current_state = STATE_CHRGAP;
            }
            else
            {
                radio_hw_dac_set(MORSE_FREQ);
                radio_hw_mode(RADIO_HW_MODE_TX);

                if (current_data & 0x01)
                {
                    current_state = STATE_DASH;
                }
                else
                {
                    current_state = STATE_DIT;
                }

                current_data = (current_data >> 1);
            }
            break;

        case STATE_STOP:
            /* wait for a dit-length gap, then start again */
            radio_hw_mode(RADIO_HW_MODE_TXOFF);
            current_state = STATE_START;
            break;

        case STATE_NSTOP:
            source_status = radio_data_update();

            if (source_status != DATA_SOURCE_OK)
            {
                return RADIO_INTERRUPT_FINISHED;
            }

            current_data = morse_get_data(radio_data_current_byte);
            current_state = STATE_START;
            break;

        default:
            current_state++;
            break;
    }

    return RADIO_INTERRUPT_OK;
}

static char morse_name[] PROGMEM = "Morse";
static PGM_P morse_getname(uint8_t t, uint8_t options)
{
    return morse_name;
}


/*
 * The alphabet below has been arranged such that it consumes much less ROM.
 * You can see how it looked before and inspect the code that generated it in
 * /misc-c/pc/tables/morse.c
 *
 * This table is 47 bytes long and covers ',' to 'Z'. Unknown items are
 * \x00, which is treated as a space.
 *
 * Each entry is a number, read from least to most significant bit. A 0 is a
 * dit, a 1 is a dash. If the item is shifted 1 bit to the right every time
 * then when it is equal to 0x01 (i.e., 00000001) then that's the end (and
 * the 1 is not a dash, it is ignored).
 *
 * For example, Z is \x13, or 0b0010011, which is dash dash dot dot stop.
 */

static uint8_t morsetab[] PROGMEM = 
    "\x73\x00\x6a\x29\x3f\x3e\x3c\x38\x30\x20\x21\x23\x27\x2f\x00\x00"
    "\x00\x00\x00\x4c\x00\x06\x11\x15\x09\x02\x14\x0b\x10\x04\x1e\x0d"
    "\x12\x07\x05\x0f\x16\x1b\x0a\x08\x03\x0c\x18\x0e\x19\x1d\x13";
#define MORSE_SPACE 0x00

static uint8_t morse_get_data(uint8_t c)
{
    if (c >= 'a' && c <= 'z')
    {
        /* Go uppercase */
        c = (c - 'a') + 'A';
    }

    if (c < ',' || c > 'Z')
    {
        return MORSE_SPACE;
    }

    c -= ',';

    return pgm_read_byte(&(morsetab[c]));
}
