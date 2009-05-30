/*
    Copyright (C) 2008  Daniel Richman

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    For a full copy of the GNU General Public License, 
    see <http://www.gnu.org/licenses/>.
*/

#ifndef ALIEN_MAIN_HEADER
#define ALIEN_MAIN_HEADER

/* Define the divide function that we use, just an alias to some libc stuff */
extern div_t udiv(int __num, int __denom) __asm__("__udivmodhi4") 
                                                               __ATTR_CONST__;

#endif 
