/* Copyright (c) 2009, Daniel Richman. All rights reserved. */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

/* Simulate timer1.c, main.c and messages.c */
#include "../final/temperature.c"
payload_message latest_data;
extern uint8_t temperature_flags, temperature_ext_crc, temperature_int_crc;

void send_char(uint8_t c)
{
  loop_until_bit_is_set(UCSR0A, UDRE0);
  UDR0 = c;
}

void send_char_hd(uint8_t c)
{
  send_char(hexdump_a(c));
  send_char(hexdump_b(c));
}

ISR (TIMER1_COMPA_vect)
{
  switch (temperature_state)
  {
    case temperature_state_null:
    case temperature_state_want_to_get:
      temperature_request();
      break;

    case temperature_state_requested:
    case temperature_state_waited:
      temperature_retrieve();
      break;
  }

  send_char_hd( ((uint8_t *) &latest_data.system_temp)[0] );
  send_char_hd( ((uint8_t *) &latest_data.system_temp)[1] );
  send_char( ' ' );
  send_char_hd( ((uint8_t *) &latest_data.system_temp)[2] );
  send_char_hd( ((uint8_t *) &latest_data.system_temp)[3] );
  send_char( ' ' );
  send_char( temperature_ext_crc );
  send_char( ' ' );
  send_char( temperature_int_crc );
  send_char( ' ' );
  send_char( temperature_flags );
  send_char( '\n' );

  latest_data.system_temp.external_temperature |= temperature_ubits_age;
  latest_data.system_temp.internal_temperature |= temperature_ubits_age;

  PORTB ^= _BV(PB5);
}

int main(void)
{
       /* Setup IO */
  DDRB  |= _BV(DDB5);     /* Put PB5 as an output (pin13) */

       /* Setup timer1 frequency as 1Hz */
  /* Prescaler to FCPU/1024; Clear timer on compare match *
   * Timer freq is 16000000/1024 = 15625Hz
   * We want 1Hz, so we want an interrupt every 15625 timer1 ticks. */
  TCCR1B  = (_BV(CS02) | _BV(CS00) | _BV(WGM12));
  OCR1A   = 15625;

       /* Initialise Temperature */
  temperature_init();

       /* Setup UART */
  UBRR0 = 207;
  UCSR0A = 0;
  UCSR0B = ((_BV(TXEN0))  | (_BV(RXEN0)));
  UCSR0C = ((_BV(UCSZ00)) | (_BV(UCSZ01)));

       /* Enable Timer1 */
  TCNT1   = 0;            /* Reset timer */
  TIMSK1  = _BV(OCIE1A);  /* Enable Compare Match Interrupts */
  sei();                  /* Turn on interrupts */

       /* Light on... */
  PORTB |= _BV(PB5);

       /* Sleep */
  for (;;) sleep_mode();
}

