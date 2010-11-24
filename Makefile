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

default : 
	$(MAKE) -C alien1/atmega162/final 
	$(MAKE) -C alien1/atmega162/tests 
	$(MAKE) -C misc-c/alien1tests-arduino-168
	$(MAKE) -C misc-c/pc
	$(MAKE) -C alien2/xmegaa4
 
clean :
	$(MAKE) -C alien1/atmega162/final clean
	$(MAKE) -C alien1/atmega162/tests clean
	$(MAKE) -C misc-c/alien1tests-arduino-168 clean
	$(MAKE) -C misc-c/pc clean
	$(MAKE) -C alien2/xmegaa4 clean

.PHONY : clean default
.DEFAULT_GOAL := default

