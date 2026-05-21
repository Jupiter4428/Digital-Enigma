#ifndef ROTOR_BW_H
#define ROTOR_BW_H

/* Circuit 6: rotor_bw  (backward rotor pass)
 *
 * Same structure as rotor_fw but uses inverse wiring tables.
 * sel: 1=Rotor I  2=Rotor II  3=Rotor III
 */
int rotor_bw(int char_in, int pos, int sel);

#endif
