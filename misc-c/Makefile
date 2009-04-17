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

V = -Dverbose
O = -g

VERBOSE =
OPT = -O2 -march=native
PEDANT = -Wall --pedantic
elffiles := $(patsubst %.c,%,$(wildcard *.c))

all : $(elffiles)
verbose : clean
	make VERBOSE=$(V) OPT=$(O) all

% : %.c
	gcc -o $@ $(OPT) $(PEDANT) $< $(VERBOSE)

clean :
	rm -f $(elffiles)

.PHONY : clean


