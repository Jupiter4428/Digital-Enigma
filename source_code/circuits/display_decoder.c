/* Circuit: display_decoder
 *
 * Logisim equivalent:
 *   ROM 32×7-bit  (addr=char_index 0–25, data=seg[6:0])
 *   Output seg[6:0] wired through Splitter → 7-Segment Display
 *
 * ROM hex data (load into Logisim ROM, addrWidth=5, dataWidth=7):
 *   addr: 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f
 *   data: 77 1f 4e 3d 4f 47 5f 37 06 3c 37 0e 76 15 3f 67
 *   addr: 10 11 12 13 14 15 16 17 18 19
 *   data: 73 05 5b 0f 3e 1c 14 37 33 6d
 *
 * Segment bit mapping: bit[6]=a  bit[5]=b  bit[4]=c  bit[3]=d
 *                      bit[2]=e  bit[1]=f  bit[0]=g
 */
#include <stdio.h>
#include "display_decoder.h"

static const unsigned char rom[26] = {
/*  A     B     C     D     E     F     G     H     I     J  */
   0x77, 0x1F, 0x4E, 0x3D, 0x4F, 0x47, 0x5F, 0x37, 0x06, 0x3C,
/*  K     L     M     N     O     P     Q     R     S     T  */
   0x37, 0x0E, 0x76, 0x15, 0x1E, 0x67, 0x73, 0x05, 0x5B, 0x0F,
/*  U     V     W     X     Y     Z  */
   0x3E, 0x1C, 0x14, 0x37, 0x33, 0x6D
};

unsigned char display_decoder(int char_index)
{
    if (char_index < 0 || char_index > 25) return 0x00;
    return rom[char_index];
}

/* Print the letter as a 3-line ASCII art 7-segment on terminal */
void display_print(int char_index)
{
    unsigned char s = display_decoder(char_index);
    int a = (s >> 6) & 1;   /* top          */
    int b = (s >> 5) & 1;   /* top-right    */
    int c = (s >> 4) & 1;   /* bottom-right */
    int d = (s >> 3) & 1;   /* bottom       */
    int e = (s >> 2) & 1;   /* bottom-left  */
    int f = (s >> 1) & 1;   /* top-left     */
    int g = (s >> 0) & 1;   /* middle       */

    /* Top segment */
    printf(" %c%c%c \n", a?'_':' ', a?'_':' ', a?'_':' ');
    /* Middle row: f, g, b */
    printf("%c%c%c%c%c\n", f?'|':' ', g?'_':' ', g?'_':' ', g?'_':' ', b?'|':' ');
    /* Bottom row: e, d, c */
    printf("%c%c%c%c%c\n", e?'|':' ', d?'_':' ', d?'_':' ', d?'_':' ', c?'|':' ');
}
