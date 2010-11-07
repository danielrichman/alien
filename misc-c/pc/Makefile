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

OPT = -O2 -march=native
CFLAGS = $(OPT) -Wall
elffiles := $(patsubst %.c,%,$(wildcard *.c tables/*.c))

all : $(elffiles)

% : %.c
	gcc $(CFLAGS) -o $@ $<

clean :
	rm -f $(elffiles)

.PHONY : clean all


