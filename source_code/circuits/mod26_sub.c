/* Circuit 2: mod26_sub
 * D = (A - B + 26) mod 26
 *
 * Logisim equivalent:
 *   C = 26 - B  (5-bit Subtractor, Constant 26)
 *   → mod26_add(A, C)     ← reuses Circuit 1 structure
 *
 * Same as: if A >= B → A-B   else → A-B+26
 */
#include "mod26_sub.h"

int mod26_sub(int a, int b)
{
    int diff = a - b;
    if (diff < 0) diff += 26;
    return diff;
}
