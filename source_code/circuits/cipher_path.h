#ifndef CIPHER_PATH_H
#define CIPHER_PATH_H

/* Circuit 8: cipher_path  (9-stage combinational datapath)
 *
 * Uses pos_next (positions AFTER stepping) — same as enigma.v
 *
 * Stage 1 : plugboard(char_in)
 * Stage 2 : rotor_fw(stage1, pos1, sel=3)   Rotor III forward
 * Stage 3 : rotor_fw(stage2, pos2, sel=2)   Rotor II  forward
 * Stage 4 : rotor_fw(stage3, pos3, sel=1)   Rotor I   forward
 * Stage 5 : reflector(stage4)
 * Stage 6 : rotor_bw(stage5, pos3, sel=1)   Rotor I   backward
 * Stage 7 : rotor_bw(stage6, pos2, sel=2)   Rotor II  backward
 * Stage 8 : rotor_bw(stage7, pos1, sel=3)   Rotor III backward
 * Stage 9 : plugboard(stage8)               → char_out
 */
int cipher_path(int char_in, int pos1, int pos2, int pos3);

#endif
