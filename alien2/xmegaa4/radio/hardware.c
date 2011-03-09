/*
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
#include <avr/interrupt.h>
#include <avr/io.h>
#include "hardware.h"
#include "radio.h"

#define RADIO_HW_MODE_PORT       PORTA
#define RADIO_DAC                DACB
#define RADIO_ADC                ADCA
#define RADIO_ADC_INPUT_AF       (ADC_CH_MUXPOS_PIN0_gc | ADC_CH_MUXPOS3_bm)
#define RADIO_ADC_INPUT_RSSI     (ADC_CH_MUXPOS_PIN1_gc | ADC_CH_MUXPOS3_bm)
#define RADIO_ADC_CAPT_PREEMPT   10
#define RADIO_HW_TIMER           TCC0
#define RADIO_HW_EVCHMUX         CH0MUX
#define RADIO_HW_EVCHSRC         EVSYS_CHMUX_TCC0_CCA_gc

static void radio_hw_dac_start();
static void radio_hw_dac_stop();
static void radio_hw_adc_init();
static void radio_hw_adc_start();
static void radio_hw_adc_stop();
static void radio_hw_timer_init();

static uint8_t radio_hw_dac_running, radio_hw_adc_running;
static uint8_t radio_hw_adc_cca_decrement;

ISR (TCC0_OVF_vect)
{
    radio_isr();
}

void radio_hw_init()
{
    radio_hw_timer_init();
    radio_hw_adc_init();
}

static void radio_hw_dac_start()
{
    RADIO_DAC.CTRLA = DAC_CH0EN_bm | DAC_ENABLE_bm;
    RADIO_DAC.CTRLC = DAC_REFSEL_INT1V_gc;
    radio_hw_dac_running = 1;
}

static void radio_hw_dac_stop()
{
    RADIO_DAC.CTRLA = 0;
    radio_hw_dac_running = 0;
}

void radio_hw_dac_set(uint16_t value)
{
    RADIO_DAC.CH0DATA = value;
}

static void radio_hw_adc_init()
{
    RADIO_ADC.REFCTRL = ADC_REFSEL_VCC_gc;
    RADIO_ADC.CH0.CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc;
    RADIO_ADC.CH0.MUXCTRL = RADIO_ADC_INPUT_AF;
    RADIO_ADC.CH1.CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc;
    RADIO_ADC.CH1.MUXCTRL = RADIO_ADC_INPUT_RSSI;
    RADIO_ADC.EVCTRL = ADC_SWEEP_01_gc | ADC_EVSEL_0123_gc |
                       ADC_EVACT_SWEEP_gc;
    RADIO_ADC.CTRLB = ADC_RESOLUTION_12BIT_gc;
}

static void radio_hw_adc_start()
{
    RADIO_ADC.CTRLA = ADC_ENABLE_bm;
    radio_hw_adc_running = 1;
}

static void radio_hw_adc_stop()
{
    RADIO_ADC.CTRLA = 0;
    radio_hw_adc_running = 0;
}

void radio_hw_adc_get(uint16_t *af, uint16_t *rssi)
{
    *af = RADIO_ADC.CH0.RES;
    *rssi = RADIO_ADC.CH1.RES;
}

static void radio_hw_timer_init()
{
    RADIO_HW_TIMER.INTCTRLA = TC_OVFINTLVL_HI_gc;
    EVSYS.RADIO_HW_EVCHMUX = RADIO_HW_EVCHSRC;
}

void radio_hw_timer_set(uint8_t div, uint16_t per)
{
    /* DIV1 = 1, DIV2 = 2, DIV4 = 3, e.t.c., so this works: */
    radio_hw_adc_cca_decrement = RADIO_ADC_CAPT_PREEMPT >> (div - 1);
    if (radio_hw_adc_cca_decrement == 0)
        radio_hw_adc_cca_decrement = 1;

    RADIO_HW_TIMER.CTRLA = 0;
    RADIO_HW_TIMER.CTRLFSET = TC_CMD_RESTART_gc;
    RADIO_HW_TIMER.PER = per;
    RADIO_HW_TIMER.CCA = per - radio_hw_adc_cca_decrement;
    RADIO_HW_TIMER.CTRLA = div;
}

void radio_hw_queue_period_update(uint16_t per)
{
    RADIO_HW_TIMER.PERBUF = per;
    RADIO_HW_TIMER.CCABUF = per - radio_hw_adc_cca_decrement;
}

/*
 * If we genuinely put the radio into idle then when we turned it back on
 * there would be a *massive* warmup time while it approaches the correct
 * frequency. This is a total pain and totally breaks hell & morse, and
 * produces an annoying warm up time for rtty and domex.
 */
#define IDLE_FREQ 4000

void radio_hw_mode(uint8_t mode)
{
    uint8_t set_idle_freq;
    set_idle_freq = 0;

    if (mode == RADIO_HW_MODE_TXOFF)
    {
        mode = RADIO_HW_MODE_TX;
        set_idle_freq = 1;
    }

    RADIO_HW_MODE_PORT.DIRCLR = RADIO_HW_MODE_BITS;

    if (mode != RADIO_HW_MODE_RX && radio_hw_adc_running)
        radio_hw_adc_stop();

    if (mode != RADIO_HW_MODE_TX && radio_hw_dac_running)
        radio_hw_dac_stop();

    if (mode == RADIO_HW_MODE_RX && !radio_hw_adc_running)
        radio_hw_adc_start();

    if (mode == RADIO_HW_MODE_TX && !radio_hw_dac_running)
        radio_hw_dac_start();

    if (set_idle_freq)
        radio_hw_dac_set(IDLE_FREQ);

    RADIO_HW_MODE_PORT.DIRSET = mode;
}
