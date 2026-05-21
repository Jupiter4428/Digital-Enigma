#include <stdio.h>
#include <string.h>
#include <ctype.h>

// =============================================================================
// ENIGMA MACHINE SIMULATOR (Translated from Verilog RTL)
// Model : Enigma I (Rotors: III -> II -> I, Reflector: B)
// =============================================================================

// ─── โครงสร้างเก็บสถานะ (เทียบเท่า Register pos1, pos2, pos3) ───
typedef struct {
    int pos1; // Rotor III (ขวาสุด)
    int pos2; // Rotor II  (กลาง)
    int pos3; // Rotor I   (ซ้ายสุด)
} EnigmaState;

// ─── Constants สำหรับ Notch ───
#define NOTCH_I   24 // Y
#define NOTCH_II  12 // M
#define NOTCH_III 21 // V

// ─── Wiring Tables (แปลงจากบล็อก case ใน Verilog) ───
// Plugboard: สลับ 5 คู่ (A<->F, B<->R, C<->X, D<->W, E<->K) ตัวอื่นผ่านปกติ
const int plugboard_map[26] = {
    5, 17, 23, 22, 10, 0, 6, 7, 8, 9, 4, 11, 12, 13, 14, 15, 16, 1, 18, 19, 20, 21, 3, 2, 24, 25
};

// Rotor I (Forward & Backward)
const int r1_fw[26] = {4, 10, 12, 5, 11, 6, 3, 16, 21, 25, 13, 19, 14, 22, 24, 7, 23, 20, 18, 15, 0, 8, 1, 17, 2, 9};
const int r1_bw[26] = {20, 22, 24, 6, 0, 3, 5, 15, 21, 25, 1, 4, 2, 10, 12, 19, 7, 23, 18, 11, 17, 8, 13, 16, 14, 9};

// Rotor II (Forward & Backward)
const int r2_fw[26] = {0, 9, 3, 10, 18, 8, 17, 20, 23, 1, 11, 7, 22, 19, 12, 2, 16, 6, 25, 13, 15, 24, 5, 21, 14, 4};
const int r2_bw[26] = {0, 9, 15, 2, 25, 22, 17, 11, 5, 1, 3, 10, 14, 19, 24, 20, 16, 6, 4, 13, 7, 23, 12, 8, 21, 18};

// Rotor III (Forward & Backward)
const int r3_fw[26] = {1, 3, 5, 7, 9, 11, 2, 15, 17, 19, 23, 21, 25, 13, 24, 4, 8, 22, 6, 0, 10, 12, 20, 18, 16, 14};
const int r3_bw[26] = {19, 0, 6, 1, 15, 2, 18, 3, 16, 4, 20, 5, 21, 13, 25, 7, 24, 8, 23, 9, 22, 11, 17, 10, 14, 12};

// Reflector B (Symmetric)
const int ref_map[26] = {24, 17, 20, 7, 16, 18, 11, 3, 15, 23, 13, 6, 14, 10, 12, 8, 4, 1, 5, 25, 2, 22, 21, 9, 0, 19};

// ─── ฟังก์ชันจำลองทางผ่านกระแสไฟของ Rotor ───
// ตรงกับ: idx_in = (char_in + pos) % 26 -> wiring -> (sub_out - pos + 26) % 26
int pass_rotor(int char_in, int pos, const int* wiring_table) {
    int idx_in = (char_in + pos) % 26;
    int sub_out = wiring_table[idx_in];
    // บวก 26 ก่อน modulo เพื่อป้องกันค่าติดลบในภาษา C (C modulo behaves differently with negative numbers)
    int char_out = (sub_out - pos + 26) % 26;
    return char_out;
}

// ─── ฟังก์ชันหลัก: รับ 1 ตัวอักษร -> กดปุ่ม -> ได้ตัวอักษรที่เข้ารหัส ───
int enigma_keypress(EnigmaState* state, int char_in) {
    // 1. Sequential: คำนวณ Notch และ Double-stepping (เทียบเท่า logic pos_next ก่อน posedge clk)
    int mid_at_notch = (state->pos2 == NOTCH_II);
    int rgt_at_notch = (state->pos1 == NOTCH_III);

    int pos1_next = (state->pos1 + 1) % 26;
    int pos2_next = state->pos2;
    int pos3_next = state->pos3;

    // เงื่อนไขโรเตอร์หมุน (Middle และ Left)
    if (rgt_at_notch || mid_at_notch) {
        pos2_next = (state->pos2 + 1) % 26;
    }
    if (mid_at_notch) {
        pos3_next = (state->pos3 + 1) % 26;
    }

    // อัปเดต Register ขยับโรเตอร์ก่อนส่งกระแสไฟวิ่ง
    state->pos1 = pos1_next;
    state->pos2 = pos2_next;
    state->pos3 = pos3_next;

    // 2. Combinational Cipher Datapath: เส้นทางกระแสไฟฟ้า
    int signal = char_in;

    // ขาไป
    signal = plugboard_map[signal];                      // ด่าน 1: Plugboard
    signal = pass_rotor(signal, state->pos1, r3_fw);     // ด่าน 2: Rotor III (fw)
    signal = pass_rotor(signal, state->pos2, r2_fw);     // ด่าน 3: Rotor II (fw)
    signal = pass_rotor(signal, state->pos3, r1_fw);     // ด่าน 4: Rotor I (fw)

    // สะท้อน
    signal = ref_map[signal];                            // ด่าน 5: Reflector B

    // ขากลับ
    signal = pass_rotor(signal, state->pos3, r1_bw);     // ด่าน 6: Rotor I (bw)
    signal = pass_rotor(signal, state->pos2, r2_bw);     // ด่าน 7: Rotor II (bw)
    signal = pass_rotor(signal, state->pos1, r3_bw);     // ด่าน 8: Rotor III (bw)
    signal = plugboard_map[signal];                      // ด่าน 9: Plugboard

    return signal;
}

// ─── ทดสอบ (Testbench) — mirrors enigma_tb.v exactly ───
// Encode then decode; show rotor positions after each press
int main(int argc, char *argv[]) {
    const char *msg = (argc > 1) ? argv[1] : "HELLO";
    int len = (int)strlen(msg);
    int encoded[256];

    EnigmaState s;

    // ── Encode ──
    s = (EnigmaState){0, 0, 0};   // init_pos1=0 init_pos2=0 init_pos3=0
    printf("\n=== Encode ===\n");
    printf("  IN -> OUT  [pos III  II   I]\n");
    for (int i = 0; i < len; i++) {
        int c = toupper((unsigned char)msg[i]) - 'A';
        if (c < 0 || c > 25) { encoded[i] = -1; continue; }
        encoded[i] = enigma_keypress(&s, c);
        printf("  %c  (%2d) -> %c  (%2d)   [%2d   %2d   %2d]\n",
               'A'+c, c, 'A'+encoded[i], encoded[i],
               s.pos1, s.pos2, s.pos3);
    }

    // ── Decode (reset then feed ciphertext back) ──
    s = (EnigmaState){0, 0, 0};
    printf("\n=== Decode (should recover original) ===\n");
    printf("  IN -> OUT  [pos III  II   I]\n");
    for (int i = 0; i < len; i++) {
        if (encoded[i] < 0) continue;
        int out = enigma_keypress(&s, encoded[i]);
        printf("  %c  (%2d) -> %c  (%2d)   [%2d   %2d   %2d]\n",
               'A'+encoded[i], encoded[i], 'A'+out, out,
               s.pos1, s.pos2, s.pos3);
    }

    printf("\n=== Done ===\n");
    return 0;
}