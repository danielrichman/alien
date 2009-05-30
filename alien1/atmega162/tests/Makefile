#!/usr/bin/make
# -*- makefile -*-

#    Copyright (C) 2008  Daniel Richman
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    For a full copy of the GNU General Public License, 
#    see <http://www.gnu.org/licenses/>.

cfiles   := $(wildcard *.c)
hexfiles := $(patsubst %.c,%.hex,$(cfiles))

all : $(hexfiles)

%.elf : %.c
	avr-gcc -mmcu=atmega168 -DF_CPU=16000000 -Wall -pedantic -O2 -o $@ $<
	avr-size $@

%.hex : %.elf
	avr-objcopy -O ihex $< $@

%.hex.upload : %.hex
	avrdude -V -F -C /etc/avrdude.conf \
	        -p atmega168 -P /dev/ttyUSB0 -c stk500v1 -b 19200 \
	        -U flash:w:$<

clean :
	rm -f *.hex 

.PHONY : clean


