#!/usr/bin/make -f
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

F_CPU = 16000000
mmcu = atmega162

cfiles   := $(wildcard *.c)
hexfiles := $(patsubst %.c,%.hex,$(cfiles))

avr_gcc = avr-gcc
avr_objcopy = avr-objcopy
avr_size = avr-size
avrdude = avrdude

CFLAGS   = -mmcu=$(mmcu) -DF_CPU=$(F_CPU) -pipe -Wall -pedantic -O2
MAPFLAGS = -Wl,-Map,

all : $(hexfiles)

%.elf : %.c
	$(avr_gcc)  -c $(CFLAGS) -o $@ $<
	$(avr_size) --common $@

%.hex : %.elf
	$(avr_objcopy) -O ihex $< $@

%.hex.upload : %.hex
	$(avrdude) -p $(mmcu) -P /dev/ttyACM0 -c stk500v2 \
	           -U flash:w:$<
	        -U flash:w:$<

clean :
	rm -f *.hex 

.PHONY : clean
.DEFAULT_GOAL := all

