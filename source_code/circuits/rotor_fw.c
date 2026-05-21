/* Circuit 5: rotor_fw  (forward rotor pass)
 *
 * Logisim equivalent:
 *   mod26_add  → idx
 *   ROM[sel]   → sub   (3 ROMs + 4:1 MUX selected by sel[1:0])
 *   mod26_sub  → char_out
 *
 * ROM hex data for Logisim:
 *
 * ROM_I_fw  (Rotor I  : EKMFLGDQVZNTOWYHXUSPAIBRCJ):
 *   04 0a 0c 05 0b 06 03 10 15 19 0d 13 0e 16 18 07 17 14 12 0f 00 08 01 11 02 09
 *
 * ROM_II_fw (Rotor II : AJDKSIRUXBLHWTMCQGZNPYFVOE):
 *   00 09 03 0a 12 08 11 14 17 01 0b 07 16 13 0c 02 10 06 19 0d 0f 18 05 15 0e 04
 *
 * ROM_III_fw(Rotor III: BDFHJLCPRTXVZNYEIWGAKMUSQO):
 *   01 03 05 07 09 0b 02 0f 11 13 17 15 19 0d 18 04 08 16 06 00 0a 0c 14 12 10 0e
 */
#include "rotor_fw.h"
#include "mod26_add.h"
#include "mod26_sub.h"

static const int rom_I[26] = {
     4,10,12, 5,11, 6, 3,16,21,25,13,19,14,22,24, 7,23,20,18,15, 0, 8, 1,17, 2, 9
};
static const int rom_II[26] = {
     0, 9, 3,10,18, 8,17,20,23, 1,11, 7,22,19,12, 2,16, 6,25,13,15,24, 5,21,14, 4
};
static const int rom_III[26] = {
     1, 3, 5, 7, 9,11, 2,15,17,19,23,21,25,13,24, 4, 8,22, 6, 0,10,12,20,18,16,14
};

int rotor_fw(int char_in, int pos, int sel)
{
    int idx = mod26_add(char_in, pos);

    int sub;
    switch (sel) {
        case 1:  sub = rom_I[idx];   break;
        case 2:  sub = rom_II[idx];  break;
        case 3:  sub = rom_III[idx]; break;
        default: sub = idx;
    }

    return mod26_sub(sub, pos);
}
