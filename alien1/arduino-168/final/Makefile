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

ANM = alien1

CFLAGS   = -mmcu=atmega168 -DF_CPU=16000000 -Wall -pedantic -O2
MAPFLAGS = -Wl,-Map,

cfiles  := $(wildcard *.c)
objects := $(patsubst %.c,%.o,$(cfiles))
headers := $(wildcard *.h)

%.o : %.c $(headers)
	avr-gcc -c $(CFLAGS) -o $@ $<

$(ANM).elf : $(objects)
	avr-gcc $(CFLAGS) -o $@ $(objects)

$(ANM).map : $(objects)
	avr-gcc $(CFLAGS) $(MAPFLAGS)$@ -o /dev/null $(objects)

analyse-memory : $(ANM).map
	@./analyse_memory $<

$(ANM).hex : $(ANM).elf
	avr-objcopy -O ihex $< $@
	avr-size $@

$(ANM).hex.upload : $(ANM).hex
	avrdude -V -F -C /etc/avrdude.conf \
	        -p atmega168 -P /dev/ttyUSB0 -c stk500v1 -b 19200 \
	        -U flash:w:$<

clean : clean-objects
	rm -f $(ANM).hex $(ANM).map

clean-objects: 
	rm -f $(ANM).elf $(objects)

.PHONY : clean clean-objects analyse-memory
.DEFAULT_GOAL := $(ANM).hex

