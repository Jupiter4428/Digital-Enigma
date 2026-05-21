/* Circuit 3: plugboard
 *
 * Historical 10-pair setting: AV BS CG DL FU HZ IN KM OW RX
 *
 * Logisim ROM data (addr/data: 6 6):
 *   addr: 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f
 *   data: 15 12 06 0b 04 14 02 19 0d 09 0c 03 0a 08 16 0f
 *   addr: 10 11 12 13 14 15 16 17 18 19
 *   data: 10 17 01 13 05 00 0e 11 18 07
 *
 * Swap pairs:
 *   A(0)↔V(21)  B(1)↔S(18)  C(2)↔G(6)   D(3)↔L(11)  F(5)↔U(20)
 *   H(7)↔Z(25)  I(8)↔N(13)  K(10)↔M(12) O(14)↔W(22) R(17)↔X(23)
 * Pass-through: E J P Q T Y
 */
#include "plugboard.h"

static const int rom[26] = {
/*  A   B  C   D   E   F  G   H   I   J   K   L   M */
   21, 18,  6, 11,  4, 20,  2, 25, 13,  9, 12,  3, 10,
/*  N   O   P   Q   R   S   T   U   V   W   X   Y   Z */
    8, 22, 15, 16, 23,  1, 19,  5,  0, 14, 17, 24,  7
};

int plugboard(int c)
{
    return rom[c];
}
