/*
    Copyright (C) 2008  Daniel Richman & Simrun Basuita

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

/* V1 takes 460 steps in gdb to execute; this version only takes 180 :D */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
  FILE *output_device;
  char target_phone[12];
  unsigned char *sms_string;
  int i, l, m;             /* Assorted uses        */
  unsigned int n;          /* and temporary values */
  int intext_length, sms_string_length;

  if (argc != 4)
  {
    printf("usage: %s </dev/device> <phone-no> <sms text>\n", argv[0]);
    exit(-1);
  }

  #define device_name   (argv[1])
  #define phoneno_input (argv[2])
  #define intext        (argv[3])

  /* Verify the Phone Number's Length */
  i = strlen(phoneno_input);

  if (i == 11 && phoneno_input[0] == '0')
  {
    /* It's a national number, starts 0 and 11 chars long */
    target_phone[0] = '4';      /* Prepend 44, english country code */
    target_phone[1] = '4';

    /* Copy other 10 chars */
    memcpy(target_phone + 2, phoneno_input + 1, 10);
  }
  else if (i == 12)
  {
    /* Its an international number without the prepended +, 
     * ready in the format that we need. Just copy */

    memcpy(target_phone, phoneno_input, 12);
  }
  else if (i == 13 && phoneno_input[0] == '+')
  {
    /* Its an international number in the format we need, but
     * we must strip the + at the start */

    memcpy(target_phone, phoneno_input + 1, 12);
  }
  else
  {
    printf("Invalid length for phone number '%s': %i\n", phoneno_input, i);
    exit(-1);
  }

  /* Now we must check that all the phone-no bytes are numeric */
  for (i = 0; i < 12; i++)
  {
    if (target_phone[i] < '0' ||
        target_phone[i] > '9')
    {
      if ( ((unsigned char) target_phone[i]) < 32 ||
           ((unsigned char) target_phone[i]) > 126)
        printf("Invalid char in phone number: 0x'%.2X'\n", 
                (unsigned char) target_phone[i]);
      else
        printf("Invalid char in phone number: '%c'\n", target_phone[i]);

      exit(-1);
    }
  }

  /* Max text length is 160. */
  intext_length = strlen(intext);

  if (intext_length > 160)
  {
    printf("Text is too long at %i chars. Max is 160.\n", i);
    exit(-1);
  }

  if (intext_length < 1)
  {
    printf("No text supplied for the SMS.\n");
    exit(-1);
  }

  /* Now we must prepare the text. We've got input in 8-bit ascii,
   * we strip the insignificant bit to get 7-bit ascii, then glue all
   * of those 7-bit chars together to get a long string of bits, which
   * we interpret as octets and hexdump. */

  /* The bitstring for the SMS contents will be 
   * (input_chars * 7) / 8 bytes long; rounded up. */
  i = intext_length * 7;       /* This is the no. of bits in the input */

  m = i % 8;                   /* Use m as a temporary variable... */
  if (m != 0)                  /* If its not an nice multiple of 8... */
    i += (8 - m);              /* Round it up. */

  sms_string_length = i / 8;   /* Then divide by 8 */

  /* Allocate the space for the text the phone will get */
  sms_string = malloc(sms_string_length);
  if (sms_string == NULL)
  {
    printf("Unable to malloc '%i' bytes for variable 'sms_string'\n", 
            sms_string_length);
    exit(-1);
  }

  /* Set every byte in sms_string to 0 */
  memset(sms_string, 0, sms_string_length);

  /* Now lets check the instring for non-ascii, and convert to sms string  */
  m = 0;      /* M will represent the byte that we are modifying           */
  l = 0;      /* L will represent how many bits we've written to that byte */

  for (i = 0; i < intext_length; i++)
  {
    if ( ((unsigned char) intext[i]) < 32 ||
         ((unsigned char) intext[i]) > 126)
    {
      printf("Invalid character in text input: 0x'%.2X'\n", 
             (unsigned char) intext[i]);
      exit(-1);
    }

    /* We want the 7 bits from 7 bit ascii. If the char passes the test above
     * we know that the most significant bit is not set. So no need to use AND
     * to filter the bits */

    /* We use the integer n as it is more than 8 bits long, so that we can
     * shift it left, past the 8 bits of a char, and thus write to two octets
     * in sms_string at once */
    n = intext[i];
    n = n << l;    /* Shift it left, align it to the next bits to be written */

    /* And now write! */
    *((unsigned int *) (&sms_string[m])) |= n;

    /* Now increment L to show that we've written 7 bits */
    l += 7;

    /* If we've gone >= 8 bits, we must move onto the next bit */
    if (l >= 8)
    {
      l -= 8;
      m++;
    }
  }

  /* Open the output device. */
  output_device = fopen(device_name, "wb");

  if (output_device == NULL)
  {
    perror("Unable to open output device");
    exit(-1);
  }

  /* OK! Lets put the Phone into PDU mode... */
  fprintf(output_device, "AT+CMGF=0\r\n");
  fflush(output_device);

  /* Ready to roll. Lets construct the AT command */
  /* The integer is the total number of octets in the hex coming up,
   * excluding the first two zeros. */
  /* The SMS-SUBMIT command is 4 octets long
   * The phone number takes up 12 chars, and is effectivly 6 octets long 
   * A PDU control string adds 3 octets
   * Another Length parameter adds 1 octet
   * Then add the length of the sms string
   * So that's 4 + 6 + 3 + 1 + sms_string_length = 
   *      14 + sms_string_length */
  fprintf(output_device, "AT+CMGS=%i\r\n", 14 + sms_string_length);
  fflush(output_device);

  /* Now we need to send the hex-encoded gory bits. */

  /* This means "send a SMS, the target no is 12 digit international */
  fprintf(output_device, "0011000C91");

  /* For the target phone number, we must swap each pair of digits */
  for (i = 0; i < 12; i += 2)        /* For each pair of digits... */
  {
    fprintf(output_device, "%c%c", 
            target_phone[i + 1],
            target_phone[i]);
  }

  /* This is a PDU string, representing the data coding scheme
   * and the protocol identifier, and how long the data is valid.
   * This is largely insignificant. */
  fprintf(output_device, "0000AA");

  /* Now the no. of chars in the text, not the no. of chars in sms_string,
   * instead intext_length */
  fprintf(output_device, "%.2X", intext_length);

  /* Now we hexdump the sms_string */
  for (i = 0; i < sms_string_length; i++)
    fprintf(output_device, "%.2X", sms_string[i]);

  /* Now we send a CTRL-Z; ASCII decimal 26 or 0x1a. */
  fprintf(output_device, "%c", 0x1a);
  fflush(output_device);

  /* All done. Close the device */
  fclose(output_device);
  free(sms_string);

  /* Finished. */
  return 0;
}
