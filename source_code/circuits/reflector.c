/* Circuit 4: reflector  (UKW-B)
 *
 * Logisim equivalent:
 *   ROM 32×5-bit  (addr=char_in, data=char_out)
 *
 * ROM hex data (load into Logisim ROM):
 *   addr: 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f
 *   data: 18 11 14 07 10 12 0b 03 0f 17 0d 06 0e 0a 0c 08
 *   addr: 10 11 12 13 14 15 16 17 18 19
 *   data: 04 01 05 19 02 16 15 09 00 13
 */
#include "reflector.h"

static const int rom[26] = {
/*  A   B   C   D   E   F   G   H   I   J   K   L   M */
   24, 17, 20,  7, 16, 18, 11,  3, 15, 23, 13,  6, 14,
/*  N   O   P   Q   R   S   T   U   V   W   X   Y   Z */
   10, 12,  8,  4,  1,  5, 25,  2, 22, 21,  9,  0, 19
};

int reflector(int c)
{
    return rom[c];
}
