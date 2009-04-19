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

CFLAGS= -mmcu=atmega168 -DF_CPU=16000000 -Wall -pedantic -O2

cfiles  := $(wildcard *.c)
objects := $(patsubst %.c,%.o,$(cfiles))
headers := $(wildcard *.h)

%.o : %.c $(headers)
	avr-gcc -c $(CFLAGS) -o $@ $<

alien1.elf: $(objects)
	avr-gcc $(CFLAGS) -o $@ $(objects)

alien1.hex : alien1.elf
	avr-objcopy -O ihex $< $@
	avr-size $@

alien1.hex.upload : alien1.hex
	avrdude -V -F -C /etc/avrdude.conf \
	        -p atmega168 -P /dev/ttyUSB0 -c stk500v1 -b 19200 \
	        -U flash:w:$<

clean : clean-objects
	rm -f alien1.hex 

clean-objects: 
	rm -f alien1.elf $(objects)

.PHONY : clean clean-objects
.DEFAULT_GOAL := alien1.hex

