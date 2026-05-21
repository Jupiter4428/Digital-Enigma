#ifndef PLUGBOARD_H
#define PLUGBOARD_H

/* Circuit 3: plugboard
 * ROM-based letter swap  (A↔F  B↔R  C↔X  D↔W  E↔K)
 * Symmetric: plugboard(plugboard(x)) == x
 */
int plugboard(int c);

#endif
