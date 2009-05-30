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
F_CPU = 16000000
mmcu = atmega162

avr_gcc = avr-gcc
avr_objcopy = avr-objcopy
avr_size = avr-size
avrdude = avrdude

CFLAGS   = -mmcu=$(mmcu) -DF_CPU=$(F_CPU) -pipe -Wall -pedantic -O2

cfiles  := $(wildcard *.c)
objects := $(patsubst %.c,%.o,$(cfiles))
headers := $(wildcard *.h)

%.o : %.c $(headers)
	$(avr_gcc) -c $(CFLAGS) -o $@ $<

$(ANM).elf : $(objects)
	$(avr_gcc)    $(CFLAGS) -o $@ $(objects)

$(ANM).hex : $(ANM).elf
	$(avr_objcopy) -O ihex $< $@
	$(avr_size) --common $(objects) $(ANM).elf $(ANM).hex

$(ANM).hex.upload : $(ANM).hex
	$(avrdude) -p $(mmcu) -P /dev/ttyACM0 -c stk500v2 \
	           -U flash:w:$<

clean : clean-objects
	rm -f $(ANM).hex

clean-objects: 
	rm -f $(ANM).elf $(objects)

.PHONY : clean clean-objects analyse-memory
.DEFAULT_GOAL := $(ANM).hex

