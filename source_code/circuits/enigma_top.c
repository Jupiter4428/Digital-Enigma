/* Circuit 9: enigma_top
 *
 * Logisim equivalent:
 *   stepping subcircuit  → POS1, POS2, POS3
 *   cipher_path subcircuit ← char_in, POS1, POS2, POS3
 *   Register (5-bit, EN=KEYPRESS) ← cipher_path output → char_out
 *   Register (1-bit, EN=1)        ← KEYPRESS           → valid_out
 *
 * Note: cipher_path receives pos_NEXT so it must be placed
 * combinationally before the stepping registers update.
 * In C we compute pos_next first, run cipher, then commit.
 */
#include "enigma_top.h"
#include "cipher_path.h"

int enigma_press(EnigmaState *s, int char_in)
{
    /* Step 1: compute pos_next (combinational, before clock edge) */
    int p1n, p2n, p3n;
    stepping_get_next(s, 1, &p1n, &p2n, &p3n);

    /* Step 2: run cipher with pos_next (combinational datapath) */
    int result = cipher_path(char_in, p1n, p2n, p3n);

    /* Step 3: commit positions (register update at posedge CLK) */
    s->pos1 = p1n;
    s->pos2 = p2n;
    s->pos3 = p3n;

    /* Step 4: return latched char_out (valid because keypress=1) */
    return result;
}
