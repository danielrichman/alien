#!/usr/bin/make
# -*- makefile -*-

#    Copyright (C) 2010  Daniel Richman
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

ANM = alien2
F_CPU = 8000000
code_mmcu = atxmega32a4
prog_mmcu = x32a4

cfiles  := $(wildcard */*.c *.c)
objects := $(patsubst %.c,%.o,$(cfiles))
headers := $(wildcard */*.h *.h)

avr_gcc = avr-gcc
avr_objcopy = avr-objcopy
avr_size = avr-size
avrdude = avrdude

CFLAGS = -mmcu=$(code_mmcu) -DF_CPU=$(F_CPU) -funsigned-char
CFLAGS += -pipe -Wall -pedantic -O2

ifdef DEBUG
    CFLAGS += -DDEBUG=$(DEBUG)
endif

all : $(hexfiles)

%.o : %.c $(headers)
	$(avr_gcc) -c $(CFLAGS) -o $@ $<

$(ANM).elf : $(objects)
	$(avr_gcc)    $(CFLAGS) -o $@ $(objects)

$(ANM).hex : $(ANM).elf
	$(avr_objcopy) -O ihex $< $@
	$(avr_size) $(objects) $(ANM).elf $(ANM).hex

$(ANM).hex.upload : $(ANM).hex
	$(avrdude) -p $(prog_mmcu) -P usb -c avrispmkii -U flash:w:$<

clean : clean-objects
	rm -f $(ANM).hex

clean-objects:
	rm -f $(ANM).elf $(objects)

.PHONY : clean clean-objects
.DEFAULT_GOAL := $(ANM).hex

