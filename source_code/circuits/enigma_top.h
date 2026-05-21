#ifndef ENIGMA_TOP_H
#define ENIGMA_TOP_H

/* Circuit 9: enigma_top
 *
 * Integrates stepping (Circuit 7) + cipher_path (Circuit 8)
 *
 * Timing (matches enigma.v posedge clk):
 *   1. Compute pos_next  (stepping_get_next)
 *   2. Run cipher with pos_next  (cipher_path)
 *   3. Commit pos_next → pos  (stepping_tick)
 *   4. Latch char_out when keypress=1
 */
#include "stepping.h"

int enigma_press(EnigmaState *s, int char_in);

#endif
