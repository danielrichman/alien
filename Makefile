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

alien1_atmega162_final_default :
	$(MAKE) -C alien1/atmega162/final 

alien1_atmega162_final_clean :
	$(MAKE) -C alien1/atmega162/final clean

alien1_atmega162_tests_default :
	$(MAKE) -C alien1/atmega162/tests 

alien1_atmega162_tests_clean : 
	$(MAKE) -C alien1/atmega162/tests clean

misc-c_default :
	$(MAKE) -C misc-c 

misc-c_clean : 
	$(MAKE) -C misc-c clean

default : alien1_atmega162_final_default alien1_atmega162_tests_default misc-c_default

clean : alien1_atmega162_final_clean alien1_atmega162_tests_clean misc-c_clean

.PHONY : clean default alien1_atmega162_final_default alien1_atmega162_tests_default misc-c_default alien1_atmega162_final_clean alien1_atmega162_tests_clean misc-c_clean

.DEFAULT_GOAL := default

