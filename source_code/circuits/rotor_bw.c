/* Circuit 6: rotor_bw  (backward rotor pass)
 *
 * Logisim equivalent: same as rotor_fw but different ROMs
 *
 * ROM hex data for Logisim:
 *
 * ROM_I_bw:
 *   14 16 18 06 00 03 05 0f 15 19 01 04 02 0a 0c 13 07 17 12 0b 11 08 0d 10 0e 09
 *
 * ROM_II_bw:
 *   00 09 0f 02 19 16 11 0b 05 01 03 0a 0e 13 18 14 10 06 04 0d 07 17 0c 08 15 12
 *
 * ROM_III_bw:
 *   13 00 06 01 0f 02 12 03 10 04 14 05 15 0d 19 07 18 08 17 09 16 0b 11 0a 0e 0c
 */
#include "rotor_bw.h"
#include "mod26_add.h"
#include "mod26_sub.h"

static const int rom_I[26] = {
    20,22,24, 6, 0, 3, 5,15,21,25, 1, 4, 2,10,12,19, 7,23,18,11,17, 8,13,16,14, 9
};
static const int rom_II[26] = {
     0, 9,15, 2,25,22,17,11, 5, 1, 3,10,14,19,24,20,16, 6, 4,13, 7,23,12, 8,21,18
};
static const int rom_III[26] = {
    19, 0, 6, 1,15, 2,18, 3,16, 4,20, 5,21,13,25, 7,24, 8,23, 9,22,11,17,10,14,12
};

int rotor_bw(int char_in, int pos, int sel)
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
