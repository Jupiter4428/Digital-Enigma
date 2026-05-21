#ifndef STEPPING_H
#define STEPPING_H

/* Circuit 7: stepping  (Double-Stepping position control)
 *
 * Mirrors enigma.v pos_next logic:
 *   NOTCH_III = 21 (V)  — Rotor III notch
 *   NOTCH_II  = 12 (M)  — Rotor II  notch
 *
 *   rgt_at_notch = (pos1 == NOTCH_III)
 *   mid_at_notch = (pos2 == NOTCH_II)
 *
 *   pos1 always steps when keypress=1
 *   pos2 steps when (rgt_at_notch OR mid_at_notch) AND keypress
 *   pos3 steps when mid_at_notch AND keypress
 */

#define NOTCH_I   24
#define NOTCH_II  12
#define NOTCH_III 21

typedef struct {
    int pos1;   /* Rotor III (right)  — POS1 */
    int pos2;   /* Rotor II  (middle) — POS2 */
    int pos3;   /* Rotor I   (left)   — POS3 */
} EnigmaState;

void stepping_reset(EnigmaState *s, int p1, int p2, int p3);
void stepping_tick(EnigmaState *s, int keypress);
void stepping_get_next(const EnigmaState *s, int keypress,
                       int *p1n, int *p2n, int *p3n);

#endif
