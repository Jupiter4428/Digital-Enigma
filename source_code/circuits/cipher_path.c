/* Circuit 8: cipher_path  (9-stage combinational datapath)
 *
 * Logisim equivalent:
 *   Place subcircuits left-to-right and wire char signal through:
 *   plugboard → rotor_fw(sel=3) → rotor_fw(sel=2) → rotor_fw(sel=1)
 *   → reflector
 *   → rotor_bw(sel=1) → rotor_bw(sel=2) → rotor_bw(sel=3) → plugboard
 *
 *   pos1 bus connects to: rotor_fw(stage2), rotor_bw(stage8)
 *   pos2 bus connects to: rotor_fw(stage3), rotor_bw(stage7)
 *   pos3 bus connects to: rotor_fw(stage4), rotor_bw(stage6)
 *
 *   IMPORTANT: pos1/pos2/pos3 must be pos_NEXT (after stepping),
 *   not current positions — Enigma steps before signal travels.
 */
#include "cipher_path.h"
#include "plugboard.h"
#include "rotor_fw.h"
#include "rotor_bw.h"
#include "reflector.h"

int cipher_path(int char_in, int pos1, int pos2, int pos3)
{
    int x = char_in;

    x = plugboard(x);             /* Stage 1: Plugboard forward  */
    x = rotor_fw(x, pos1, 3);    /* Stage 2: Rotor III forward  */
    x = rotor_fw(x, pos2, 2);    /* Stage 3: Rotor II  forward  */
    x = rotor_fw(x, pos3, 1);    /* Stage 4: Rotor I   forward  */
    x = reflector(x);             /* Stage 5: Reflector B        */
    x = rotor_bw(x, pos3, 1);    /* Stage 6: Rotor I   backward */
    x = rotor_bw(x, pos2, 2);    /* Stage 7: Rotor II  backward */
    x = rotor_bw(x, pos1, 3);    /* Stage 8: Rotor III backward */
    x = plugboard(x);             /* Stage 9: Plugboard backward */

    return x;
}
