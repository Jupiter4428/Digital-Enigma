#ifndef DISPLAY_DECODER_H
#define DISPLAY_DECODER_H

/* Circuit: display_decoder
 * Input : char_index  (0=A … 25=Z)
 * Output: 7-bit segment code  bit[6]=a … bit[0]=g  (1=ON)
 *
 * 7-Segment layout:
 *      aaa
 *     f   b
 *     f   b
 *      ggg
 *     e   c
 *     e   c
 *      ddd
 */
unsigned char display_decoder(int char_index);

/* Print the letter as ASCII art on terminal (3 lines) */
void display_print(int char_index);

#endif
