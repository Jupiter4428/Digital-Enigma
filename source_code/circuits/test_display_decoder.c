/* test_display_decoder.c — Unit tests for display_decoder
 *
 * Compile:
 *   gcc -Wall -o test_display_decoder test_display_decoder.c display_decoder.c
 *
 * Run:
 *   ./test_display_decoder
 */
#include <stdio.h>
#include "display_decoder.h"

static int tests_run    = 0;
static int tests_failed = 0;

#define ASSERT_EQ(label, got, expected)                                         \
    do {                                                                        \
        tests_run++;                                                            \
        if ((got) != (expected)) {                                              \
            printf("FAIL  %s: got 0x%02X, expected 0x%02X\n",                 \
                   (label), (unsigned)(got), (unsigned)(expected));             \
            tests_failed++;                                                     \
        } else {                                                                \
            printf("PASS  %s\n", (label));                                      \
        }                                                                       \
    } while (0)

/* ── 1. Known ROM values (ตรวจทุกตัวอักษร) ──────────────────────────── */
static void test_known_values(void)
{
    printf("\n--- Known ROM values (all 26 letters) ---\n");

    /* ROM data จาก display_decoder.c */
    static const unsigned char expected[26] = {
        0x77, 0x1F, 0x4E, 0x3D, 0x4F, 0x47, 0x5F, 0x37, 0x06, 0x3C,
        0x37, 0x0E, 0x76, 0x15, 0x3F, 0x67, 0x73, 0x05, 0x5B, 0x0F,
        0x3E, 0x1C, 0x14, 0x37, 0x33, 0x6D
    };

    static const char *letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char label[16];
    for (int i = 0; i < 26; i++) {
        snprintf(label, sizeof(label), "%c(%2d)=0x%02X", letters[i], i, expected[i]);
        ASSERT_EQ(label, display_decoder(i), expected[i]);
    }
}

/* ── 2. ตรวจ bit mapping ของ segment แต่ละอัน ───────────────────────── */
/*   bit[6]=a  bit[5]=b  bit[4]=c  bit[3]=d  bit[2]=e  bit[1]=f  bit[0]=g  */

static void check_segments(const char *letter, int idx,
                            int a, int b, int c, int d, int e, int f, int g)
{
    unsigned char seg = display_decoder(idx);
    int got_a = (seg >> 6) & 1;
    int got_b = (seg >> 5) & 1;
    int got_c = (seg >> 4) & 1;
    int got_d = (seg >> 3) & 1;
    int got_e = (seg >> 2) & 1;
    int got_f = (seg >> 1) & 1;
    int got_g = (seg >> 0) & 1;

    char label[32];
    snprintf(label, sizeof(label), "%s seg-a", letter); ASSERT_EQ(label, got_a, a);
    snprintf(label, sizeof(label), "%s seg-b", letter); ASSERT_EQ(label, got_b, b);
    snprintf(label, sizeof(label), "%s seg-c", letter); ASSERT_EQ(label, got_c, c);
    snprintf(label, sizeof(label), "%s seg-d", letter); ASSERT_EQ(label, got_d, d);
    snprintf(label, sizeof(label), "%s seg-e", letter); ASSERT_EQ(label, got_e, e);
    snprintf(label, sizeof(label), "%s seg-f", letter); ASSERT_EQ(label, got_f, f);
    snprintf(label, sizeof(label), "%s seg-g", letter); ASSERT_EQ(label, got_g, g);
}

static void test_segments(void)
{
    printf("\n--- Segment bits per letter ---\n");

    /*  A = 0x77 = 1110111  →  a=1 b=1 c=1 d=0 e=1 f=1 g=1
     *   ___
     *  |___|
     *  |   |
     */
    check_segments("A", 0,  /*a*/1,/*b*/1,/*c*/1,/*d*/0,/*e*/1,/*f*/1,/*g*/1);

    /*  E = 0x4F = 1001111  →  a=1 b=0 c=0 d=1 e=1 f=1 g=1
     *   ___
     *  |___
     *  |___
     */
    check_segments("E", 4,  /*a*/1,/*b*/0,/*c*/0,/*d*/1,/*e*/1,/*f*/1,/*g*/1);

    /*  O = 0x3F = 0111111  →  a=0 b=1 c=1 d=1 e=1 f=1 g=1
     *      (no top segment)
     *  |   |
     *  |___|
     */
    check_segments("O", 14, /*a*/0,/*b*/1,/*c*/1,/*d*/1,/*e*/1,/*f*/1,/*g*/1);

    /*  I = 0x06 = 0000110  →  a=0 b=1 c=1 d=0 e=0 f=0 g=0 */
    check_segments("I", 8,  /*a*/0,/*b*/1,/*c*/1,/*d*/0,/*e*/0,/*f*/0,/*g*/0);

    /*  Z = 0x6D = 1101101  →  a=1 b=1 c=0 d=1 e=0 f=1 g=0  (wait: 0x6D=1101101)
     *  bit6=1(a) bit5=1(b) bit4=0(c) bit3=1(d) bit2=1(e) bit1=0(f) bit0=1(g) */
    check_segments("Z", 25, /*a*/1,/*b*/1,/*c*/0,/*d*/1,/*e*/1,/*f*/0,/*g*/1);
}

/* ── 3. Output อยู่ใน range 0x00–0x7F (7-bit) ────────────────────────── */
static void test_output_range(void)
{
    printf("\n--- Output in 7-bit range [0x00, 0x7F] ---\n");
    int failed_any = 0;
    for (int i = 0; i < 26; i++) {
        unsigned char seg = display_decoder(i);
        tests_run++;
        if (seg > 0x7F) {
            printf("FAIL  display_decoder(%d) = 0x%02X > 0x7F\n", i, seg);
            tests_failed++;
            failed_any = 1;
        }
    }
    if (!failed_any)
        printf("PASS  all 26 outputs are 7-bit values\n");
}

/* ── 4. ไม่มีตัวอักษรไหน output = 0x00 (ทุกตัวต้องมี segment ติดอย่างน้อย 1) ── */
static void test_nonzero(void)
{
    printf("\n--- No letter maps to all-segments-off (0x00) ---\n");
    int failed_any = 0;
    for (int i = 0; i < 26; i++) {
        tests_run++;
        if (display_decoder(i) == 0x00) {
            printf("FAIL  display_decoder(%d='%c') = 0x00\n", i, 'A'+i);
            tests_failed++;
            failed_any = 1;
        }
    }
    if (!failed_any)
        printf("PASS  all 26 letters have at least one segment ON\n");
}

/* ── 5. Input นอก range 0-25 ต้องคืน 0x00 ─────────────────────────────── */
static void test_out_of_range(void)
{
    printf("\n--- Out-of-range inputs return 0x00 ---\n");
    ASSERT_EQ("display_decoder(-1)", display_decoder(-1), 0x00);
    ASSERT_EQ("display_decoder(26)", display_decoder(26), 0x00);
    ASSERT_EQ("display_decoder(31)", display_decoder(31), 0x00);
}

/* ── 6. Visual: แสดง ASCII art 7-segment ทุกตัว ─────────────────────── */
static void test_visual(void)
{
    printf("\n--- Visual: 7-segment ASCII art (A–Z) ---\n");
    for (int i = 0; i < 26; i++) {
        printf("  %c:\n", 'A' + i);
        display_print(i);
        printf("\n");
    }
}

int main(void)
{
    printf("========== display_decoder tests ==========\n");

    test_known_values();
    test_segments();
    test_output_range();
    test_nonzero();
    test_out_of_range();
    test_visual();

    printf("========== Results: %d/%d passed",
           tests_run - tests_failed, tests_run);
    if (tests_failed == 0)
        printf(" — ALL PASSED ==========\n");
    else
        printf(" — %d FAILED ==========\n", tests_failed);

    return tests_failed ? 1 : 0;
}
