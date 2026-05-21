#ifndef ROTOR_FW_H
#define ROTOR_FW_H

/* Circuit 5: rotor_fw  (forward rotor pass)
 *
 * Logic (from enigma.v):
 *   idx      = mod26_add(char_in, pos)   [Circuit 1]
 *   sub      = ROM_fw[sel][idx]          [ROM lookup]
 *   char_out = mod26_sub(sub, pos)       [Circuit 2]
 *
 * sel: 1=Rotor I  2=Rotor II  3=Rotor III
 */
int rotor_fw(int char_in, int pos, int sel);

#endif
