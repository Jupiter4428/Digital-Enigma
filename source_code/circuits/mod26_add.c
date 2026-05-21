/* Circuit 1: mod26_add
 * S = (A + B) mod 26
 *
 * Logisim equivalent:
 *   Bit Extender(A 5→6) + Bit Extender(B 5→6)
 *   → 6-bit Adder (sum = A+B, max=50)
 *   → Comparator (sum >= 26?)
 *   → MUX: sum>=26 ? sum-26 : sum
 *   → Bit Selector [4:0]
 */
#include "mod26_add.h"

int mod26_add(int a, int b)
{
    int sum = a + b;
    if (sum >= 26) sum -= 26;
    return sum;
}
