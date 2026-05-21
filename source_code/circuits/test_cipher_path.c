/* test_cipher_path.c — Unit tests for Circuit 8: cipher_path
 *
 * Compile:
 *   gcc -Wall -o test_cipher_path \
 *       test_cipher_path.c mod26_add.c mod26_sub.c plugboard.c \
 *       reflector.c rotor_fw.c rotor_bw.c cipher_path.c
 *
 * Run:
 *   ./test_cipher_path
 */
#include <stdio.h>
#include "cipher_path.h"

static int tests_run    = 0;
static int tests_failed = 0;

#define ASSERT_EQ(label, got, expected)                                     \
    do {                                                                    \
        tests_run++;                                                        \
        if ((got) != (expected)) {                                          \
            printf("FAIL  %s: got %d ('%c'), expected %d ('%c')\n",        \
                   (label), (got), 'A'+(got), (expected), 'A'+(expected)); \
            tests_failed++;                                                 \
        } else {                                                            \
            printf("PASS  %s\n", (label));                                  \
        }                                                                   \
    } while (0)

/* ── 1. Known values (hand-traced through all 9 stages) ──────────────── */
static void test_known_values(void)
{
    printf("\n--- Known values (pos1=0,pos2=0,pos3=0) ---\n");

    /* A(0)  → T(19)  — verified by hand */
    ASSERT_EQ("A->T  (0,0,0,0)", cipher_path(0, 0, 0, 0), 19);
    /* T(19) → A(0)   — confirms self-inverse */
    ASSERT_EQ("T->A  (19,0,0,0)", cipher_path(19, 0, 0, 0), 0);

    /* B(1)  → M(12) */
    ASSERT_EQ("B->M  (1,0,0,0)", cipher_path(1, 0, 0, 0), 12);
    /* M(12) → B(1)  */
    ASSERT_EQ("M->B  (12,0,0,0)", cipher_path(12, 0, 0, 0), 1);

    printf("\n--- Known values (pos1=1,pos2=2,pos3=3) ---\n");

    /* A(0)  → J(9) */
    ASSERT_EQ("A->J  (0,1,2,3)", cipher_path(0, 1, 2, 3), 9);
    /* J(9)  → A(0) */
    ASSERT_EQ("J->A  (9,1,2,3)", cipher_path(9, 1, 2, 3), 0);
}

/* ── 2. Involution: cipher_path(cipher_path(c,p1,p2,p3),p1,p2,p3) == c ─ */
static void test_involution(int p1, int p2, int p3)
{
    char label[64];
    for (int c = 0; c < 26; c++) {
        int mid = cipher_path(c, p1, p2, p3);
        int out = cipher_path(mid, p1, p2, p3);
        snprintf(label, sizeof(label),
                 "involution %c pos(%d,%d,%d)", 'A'+c, p1, p2, p3);
        ASSERT_EQ(label, out, c);
    }
}

static void test_all_involution(void)
{
    printf("\n--- Involution: all 26 letters at pos (0,0,0) ---\n");
    test_involution(0, 0, 0);

    printf("\n--- Involution: all 26 letters at pos (1,0,0) ---\n");
    test_involution(1, 0, 0);

    printf("\n--- Involution: all 26 letters at pos (0,1,0) ---\n");
    test_involution(0, 1, 0);

    printf("\n--- Involution: all 26 letters at pos (0,0,1) ---\n");
    test_involution(0, 0, 1);

    printf("\n--- Involution: all 26 letters at pos (25,25,25) ---\n");
    test_involution(25, 25, 25);

    printf("\n--- Involution: all 26 letters at pos (13,7,19) ---\n");
    test_involution(13, 7, 19);
}

/* ── 3. Output always in [0, 25] ─────────────────────────────────────── */
static void test_output_range(void)
{
    printf("\n--- Output range [0,25] ---\n");
    int failed_any = 0;
    for (int p1 = 0; p1 < 26; p1++) {
        for (int p2 = 0; p2 < 26; p2 += 7) {        /* sample pos2 */
            for (int p3 = 0; p3 < 26; p3 += 7) {    /* sample pos3 */
                for (int c = 0; c < 26; c++) {
                    int out = cipher_path(c, p1, p2, p3);
                    tests_run++;
                    if (out < 0 || out > 25) {
                        printf("FAIL  range: cipher_path(%d,%d,%d,%d) = %d\n",
                               c, p1, p2, p3, out);
                        tests_failed++;
                        failed_any = 1;
                    }
                }
            }
        }
    }
    if (!failed_any)
        printf("PASS  all sampled outputs in [0,25]\n");
}

/* ── 4. No fixed points (Enigma never encodes a letter to itself) ─────── */
static void test_no_fixed_points(void)
{
    printf("\n--- No fixed points ---\n");
    int failed_any = 0;
    for (int p1 = 0; p1 < 26; p1++) {
        for (int p2 = 0; p2 < 26; p2++) {
            for (int p3 = 0; p3 < 26; p3++) {
                for (int c = 0; c < 26; c++) {
                    int out = cipher_path(c, p1, p2, p3);
                    tests_run++;
                    if (out == c) {
                        printf("FAIL  fixed point: cipher_path(%d,%d,%d,%d) = %d\n",
                               c, p1, p2, p3, out);
                        tests_failed++;
                        failed_any = 1;
                    }
                }
            }
        }
    }
    if (!failed_any)
        printf("PASS  no fixed points across all 26^4 combinations\n");
}

/* ── 5. Reciprocal: cipher_path(a)==b implies cipher_path(b)==a ────────── */
static void test_reciprocal(void)
{
    printf("\n--- Reciprocal pairs at pos (0,0,0) ---\n");
    for (int a = 0; a < 26; a++) {
        int b = cipher_path(a, 0, 0, 0);
        char label[64];
        snprintf(label, sizeof(label), "recip %c<->%c", 'A'+a, 'A'+b);
        ASSERT_EQ(label, cipher_path(b, 0, 0, 0), a);
    }
}

int main(void)
{
    printf("========== cipher_path tests ==========\n");

    test_known_values();
    test_all_involution();
    test_output_range();
    test_no_fixed_points();
    test_reciprocal();

    printf("\n========== Results: %d/%d passed",
           tests_run - tests_failed, tests_run);
    if (tests_failed == 0)
        printf(" — ALL PASSED ==========\n");
    else
        printf(" — %d FAILED ==========\n", tests_failed);

    return tests_failed ? 1 : 0;
}
