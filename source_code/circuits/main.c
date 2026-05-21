/* main.c — Testbench (mirrors enigma_tb.v)
 *
 * Compile (all circuits):
 *   gcc -Wall -o enigma \
 *       main.c mod26_add.c mod26_sub.c plugboard.c reflector.c \
 *       rotor_fw.c rotor_bw.c stepping.c cipher_path.c enigma_top.c \
 *       display_decoder.c
 *
 * Usage:
 *   ./enigma HELLO
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "enigma_top.h"
#include "display_decoder.h"

/* Print one encode/decode row with 7-segment display side-by-side */
static void print_row(int in, int out, int p1, int p2, int p3)
{
    /* Collect 3 display lines for input letter */
    char in_lines[3][8], out_lines[3][8];
    unsigned char si = display_decoder(in);
    unsigned char so = display_decoder(out);

    int ai = (si>>6)&1, bi = (si>>5)&1, ci = (si>>4)&1;
    int di = (si>>3)&1, ei = (si>>2)&1, fi = (si>>1)&1, gi = (si>>0)&1;
    int ao = (so>>6)&1, bo = (so>>5)&1, co = (so>>4)&1;
    int do_ = (so>>3)&1, eo = (so>>2)&1, fo = (so>>1)&1, go = (so>>0)&1;

    /* Build ASCII art strings */
    snprintf(in_lines[0], sizeof(in_lines[0]),  " %c%c%c ", ai?'_':' ', ai?'_':' ', ai?'_':' ');
    snprintf(in_lines[1], sizeof(in_lines[1]),  "%c%c%c%c%c", fi?'|':' ', gi?'_':' ', gi?'_':' ', gi?'_':' ', bi?'|':' ');
    snprintf(in_lines[2], sizeof(in_lines[2]),  "%c%c%c%c%c", ei?'|':' ', di?'_':' ', di?'_':' ', di?'_':' ', ci?'|':' ');

    snprintf(out_lines[0], sizeof(out_lines[0]), " %c%c%c ", ao?'_':' ', ao?'_':' ', ao?'_':' ');
    snprintf(out_lines[1], sizeof(out_lines[1]), "%c%c%c%c%c", fo?'|':' ', go?'_':' ', go?'_':' ', go?'_':' ', bo?'|':' ');
    snprintf(out_lines[2], sizeof(out_lines[2]), "%c%c%c%c%c", eo?'|':' ', do_?'_':' ', do_?'_':' ', do_?'_':' ', co?'|':' ');

    /* Print: 7-seg IN  →  7-seg OUT  [positions] */
    printf("  %s     %s\n",  in_lines[0], out_lines[0]);
    printf("  %s  →  %s   [III=%2d  II=%2d  I=%2d]\n",
           in_lines[1], out_lines[1], p1, p2, p3);
    printf("  %s     %s\n",  in_lines[2], out_lines[2]);
    printf("   %c(%2d)      %c(%2d)\n", 'A'+in, in, 'A'+out, out);
    printf("\n");
}

int main(int argc, char *argv[])
{
    const char *msg = (argc > 1) ? argv[1] : "HELLO";
    int len = (int)strlen(msg);
    int encoded[256];

    EnigmaState s;

    /* ── Encode ── */
    stepping_reset(&s, 0, 0, 0);
    printf("\n========== Encode ==========\n\n");
    for (int i = 0; i < len; i++) {
        int c = toupper((unsigned char)msg[i]) - 'A';
        if (c < 0 || c > 25) { encoded[i] = -1; continue; }
        encoded[i] = enigma_press(&s, c);
        print_row(c, encoded[i], s.pos1, s.pos2, s.pos3);
    }

    /* ── Decode ── */
    stepping_reset(&s, 0, 0, 0);
    printf("========== Decode (should recover original) ==========\n\n");
    for (int i = 0; i < len; i++) {
        if (encoded[i] < 0) continue;
        int out = enigma_press(&s, encoded[i]);
        print_row(encoded[i], out, s.pos1, s.pos2, s.pos3);
    }

    printf("========== Done ==========\n");
    return 0;
}
