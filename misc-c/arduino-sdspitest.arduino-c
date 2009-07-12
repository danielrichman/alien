/*
    Copyright (C) 2008  Daniel Richman & Simrun Basuita

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

/* mv arduino-sdspitest.arduino-c arduino-sdspitest.arduino.c &&
   avr-gcc -mmcu=atmega168 -DF_CPU=16000000 -Wall -pedantic -O2 -o spitest.elf arduino-sdspitest.arduino.c && 
   mv arduino-sdspitest.arduino.c arduino-sdspitest.arduino-c &&
   avr-objcopy -O ihex spitest.elf spitest.hex &&
   avr-size spitest.hex &&
   avrdude -V -F -C /etc/avrdude.conf \
		-p atmega168 -P /dev/ttyUSB0 -c stk500v1 -b 19200 \
		-U flash:w:spitest.hex &&
   rm spitest.elf spitest.hex &&
   stty -F /dev/ttyUSB0 cs8 115200 ignbrk -brkint -icrnl -imaxbel -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke noflsh -ixon -crtscts && 
   cat /dev/ttyUSB0 | tee dump.tmp */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#define SS_HIGH  PORTB |=   _BV(PB2)
#define SS_LOW   PORTB &= ~(_BV(PB2))

#define num_to_char(number)   ((number) < 10 ? ('0' + (number)) : (('A' - 10) + (number)) )

#define first_four(byte)       (0x0F & (byte))
#define  last_four(byte)      ((0xF0 & (byte)) >> 4)

#define hexdump_a(byte)  num_to_char( last_four(byte))
#define hexdump_b(byte)  num_to_char(first_four(byte))

void send_char(uint8_t c)
{
  loop_until_bit_is_set(UCSR0A, UDRE0);
  UDR0 = c;
}

uint8_t spi_byte(uint8_t b)
{
  uint8_t r;

  SPDR = b;

  send_char(hexdump_a(b));
  send_char(hexdump_b(b));
  send_char(' ');

  loop_until_bit_is_set(SPSR, SPIF);
  r = SPDR;

  send_char(hexdump_a(r));
  send_char(hexdump_b(r));
  send_char('\n');

  return r;
}

uint8_t crc7_byte_update(uint8_t crc, uint8_t b)
{
  uint8_t i;

  for (i = 0x80; i != 0; i = i >> 1)
  {
    crc = crc << 1;

    if ((b & i) || (crc & 0x80))
    {
      if (!((b & i) && (crc & 0x80)))
      {
        crc ^= 0x09;
      }
    }
  }

  return crc & 0x7f;
}

int main(void)
{
  uint8_t  i;
  uint16_t j;

  cli();

  DDRB  |= ((_BV(PB2)) | (_BV(PB3)) | (_BV(PB5)));
  PORTB |= ((_BV(PB4)));
  SPCR   = ((_BV(SPE)) | (_BV(MSTR)) | (_BV(SPR0)) | (_BV(SPR1)));
  UCSR0B = ((_BV(TXEN0)));
  UBRR0  = 8;

  i = SPSR;
  i = SPDR;

  SS_HIGH;

  for (i = 0; i < 10; i++) spi_byte(0xFF);

  SS_LOW;

  spi_byte(0x40 | 0);
  spi_byte(0x00);
  spi_byte(0x00);
  spi_byte(0x00);
  spi_byte(0x00);
  spi_byte(0x95);
 
  j = 0;

  do
  {
    i = spi_byte(0xFF);
    j++;
  }
  while (i == 0xFF && j < 0x1000);

  if (i != 0x01)  for (;;) sleep_mode();

  spi_byte(0x40 | 8);
  spi_byte(0x00);
  spi_byte(0x00);
  spi_byte(0x01);
  spi_byte(0xAA);
  spi_byte(0x87);

  while (spi_byte(0xFF) == 0xFF);
  for (i = 0; i < 6; i++) spi_byte(0xFF);

  do
  {
    spi_byte(0x40 | 1);
    spi_byte(0x00);
    spi_byte(0x00);
    spi_byte(0x00);
    spi_byte(0x00);
    spi_byte(0x00);

    do
    {
      i = spi_byte(0xFF);
    }
    while (i == 0xFF);
  }
  while (i == 0x01);

  spi_byte(0x40 | 16);
  spi_byte(0x00);
  spi_byte(0x00);
  spi_byte(0x00);
  spi_byte(0x80);
  spi_byte(0x00);

  while (spi_byte(0xFF) == 0xFF);
  for (i = 0; i < 6; i++) spi_byte(0xFF);

  spi_byte(0x40 | 17);
  spi_byte(0x00);
  spi_byte(0x00);
  spi_byte(0x00);
  spi_byte(0x00);
  spi_byte(0x00);

  while (spi_byte(0xFF) == 0xFF);
  for (j = 0; j < 150; j++) spi_byte(0xFF);

  spi_byte(0x40 | 24);
  spi_byte(0x00);
  spi_byte(0x00);
  spi_byte(0x00);
  spi_byte(0x00);
  spi_byte(0x00);

  while (spi_byte(0xFF) == 0xFF);
  spi_byte(0xFE);
  for (j = 0; j < 128; j++) spi_byte(0x55);
  for (i = 0; i < 6;   i++) spi_byte(0xFF);
  while (spi_byte(0xFF) != 0xFF);

  spi_byte(0x40 | 13);
  spi_byte(0x00);
  spi_byte(0x00);
  spi_byte(0x00);
  spi_byte(0x00);
  spi_byte(0x00);

  while (spi_byte(0xFF) == 0xFF);
  for (i = 0; i < 6; i++) spi_byte(0xFF);

  spi_byte(0x40 | 17);
  spi_byte(0x00);
  spi_byte(0x00);
  spi_byte(0x00);
  spi_byte(0x00);
  spi_byte(0x00);

  while (spi_byte(0xFF) == 0xFF);
  for (j = 0; j < 150; j++) spi_byte(0xFF);

  SS_HIGH;

  spi_byte(0xFF);

  for (;;) sleep_mode();

  return 0;
}

