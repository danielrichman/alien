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

/* INSTRUCTIONS */
/* Step 1) Ensure that you have copied phone_no.h to phone_no_private.h
 * Step 2) The phone number digits are mangled a bit. Each digit must go in
 *         a separate #define, then we can take care of the rest.
 * Step 3) Delete the #error line below. */

#error DELETE ME AFTER EDITING - phone_no_private.h

#ifndef ALIEN_PHONE_NO_PRIV_HEADER
#define ALIEN_PHONE_NO_PRIV_HEADER

  /* BEGIN EDITING HERE */
  /* Example phone number is +441234567890 */
  #define ALIEN_PHONE_NO_11    4
  #define ALIEN_PHONE_NO_12    4
  #define ALIEN_PHONE_NO_21    1
  #define ALIEN_PHONE_NO_22    2
  #define ALIEN_PHONE_NO_31    3
  #define ALIEN_PHONE_NO_32    4
  #define ALIEN_PHONE_NO_41    5
  #define ALIEN_PHONE_NO_42    6
  #define ALIEN_PHONE_NO_51    7
  #define ALIEN_PHONE_NO_52    8
  #define ALIEN_PHONE_NO_61    9
  #define ALIEN_PHONE_NO_62    0
  /* STOP EDITING HERE */

#else

