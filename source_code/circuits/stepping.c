/* Circuit 7: stepping  (Double-Stepping position control)
 *
 * Logisim equivalent:
 *   3× Counter (5-bit, max=25)
 *   2× Comparator  (POS1==21, POS2==12)
 *   1× OR Gate     (rgt_notch OR mid_notch)
 *   2× AND Gate    (OR_out AND KEYPRESS → EN Rotor_II)
 *                  (mid_notch AND KEYPRESS → EN Rotor_I)
 *   1× NOT Gate    (RST active-low → CLR active-high)
 *
 * stepping_tick() = one posedge CLK with given keypress value
 * stepping_get_next() = compute pos_next without committing
 *   (used by cipher_path to get pos_next before register update)
 */
#include "stepping.h"

static int mod26_inc(int x) { return (x >= 25) ? 0 : x + 1; }

void stepping_reset(EnigmaState *s, int p1, int p2, int p3)
{
    s->pos1 = p1;
    s->pos2 = p2;
    s->pos3 = p3;
}

void stepping_get_next(const EnigmaState *s, int keypress,
                       int *p1n, int *p2n, int *p3n)
{
    int rgt_at_notch = (s->pos1 == NOTCH_III);
    int mid_at_notch = (s->pos2 == NOTCH_II);

    *p1n = keypress ? mod26_inc(s->pos1) : s->pos1;
    *p2n = (keypress && (rgt_at_notch || mid_at_notch)) ? mod26_inc(s->pos2) : s->pos2;
    *p3n = (keypress && mid_at_notch)                   ? mod26_inc(s->pos3) : s->pos3;
}

void stepping_tick(EnigmaState *s, int keypress)
{
    int p1n, p2n, p3n;
    stepping_get_next(s, keypress, &p1n, &p2n, &p3n);
    s->pos1 = p1n;
    s->pos2 = p2n;
    s->pos3 = p3n;
}
