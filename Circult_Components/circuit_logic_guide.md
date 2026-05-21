# Enigma Machine — Circuit Logic Guide
## หน้าที่ของแต่ละ Logic Component ในแต่ละวงจร

---

## Circuit 1 — `mod26_add` (S = (A + B) mod 26)

**หน้าที่โดยรวม:** คำนวณการบวกสองตัวเลขในระบบ mod 26 — ใช้ทุกครั้งที่โรเตอร์ต้องบวก offset เข้ากับ char_in เพื่อหา idx สำหรับ lookup ROM และทุกครั้งที่ mod26_sub ต้องแปลงการลบเป็นการบวก วงจรนี้รับรองว่าผลลัพธ์อยู่ในช่วง 0–25 เสมอ แม้ผลบวกจะเกิน 25

```
flow: A[6] + B[6] → sum → ถ้า sum ≥ 26 → sum-26 → S[6]
                         ถ้า sum < 26 → sum → S[6]
```

### Bit Extender (×2)
**หน้าที่:** รับ input 6-bit และส่งต่อโดยไม่เปลี่ยนค่า (no-op) เพื่อรักษา layout ของวงจร

| A (6-bit) | Bit Extender output | หมายเหตุ |
|-----------|-------------------|---------|
| 000111 (7) | 000111 (7) | ผ่านตรง |
| 011001 (25) | 011001 (25) | ผ่านตรง |
| 000000 (0) | 000000 (0) | ผ่านตรง |

---

### Adder (6-bit)
**หน้าที่:** บวก A + B แบบ binary ผลลัพธ์อาจเกิน 25 → ต้องมี mod ตามมา

| A | B | Adder output | เกิน 25? |
|---|---|-------------|--------|
| 7 | 1 | 8 | ไม่ |
| 25 | 1 | 26 | **ใช่** → ต้องลบ 26 |
| 13 | 13 | 26 | **ใช่** → ต้องลบ 26 |

---

### Comparator (6-bit, unsigned, A≥B)
**หน้าที่:** ตรวจสอบว่า sum ≥ 26 หรือไม่ → ถ้าใช่ให้ MUX เลือก sum-26 แทน

| Adder sum | Constant B (26) | A≥B output | MUX จะเลือก |
|-----------|----------------|-----------|-----------|
| 25 | 26 | 0 (LOW) | sum ตรงๆ |
| 26 | 26 | 1 (HIGH) | sum - 26 |
| 50 | 26 | 1 (HIGH) | sum - 26 |

---

### Constant (×2, value=26)
**หน้าที่:** กำหนดค่า modulus คงที่ = 26 ให้ Comparator และ Subtractor

| Constant | ต่อกับ | หน้าที่ |
|---------|--------|--------|
| 26 | Comparator B | เกณฑ์เปรียบเทียบ |
| 26 | Subtractor B | ค่าที่ลบออก |
| 26 | ทั้งสอง | ไม่เปลี่ยนตลอด operation |

---

### Subtractor (6-bit)
**หน้าที่:** คำนวณ sum - 26 สำหรับกรณีที่ sum ≥ 26

| sum | 26 | Subtractor output | ใช้จริง? |
|-----|-----|-----------------|--------|
| 26 | 26 | 0 | ✓ (26 mod 26 = 0) |
| 27 | 26 | 1 | ✓ |
| 25 | 26 | -1 (ไม่ถูก) | ✗ MUX ไม่เลือก |

---

### Multiplexer (6-bit, 2:1)
**หน้าที่:** เลือกผลลัพธ์สุดท้าย — ถ้า sum < 26 ให้ sum, ถ้า sum ≥ 26 ให้ sum-26

| Comparator A≥B | input 0 (sum) | input 1 (sum-26) | MUX output (S) |
|---------------|--------------|----------------|---------------|
| 0 | 8 | -18 (ไม่ใช้) | **8** |
| 1 | 26 | 0 | **0** |
| 1 | 50 | 24 | **24** |

---

## Circuit 2 — `mod26_sub` (D = (A − B + 26) mod 26)

**หน้าที่โดยรวม:** คำนวณการลบสองตัวเลขในระบบ mod 26 — ใช้ทุกครั้งที่โรเตอร์ต้องลบ pos ออกจาก sub เพื่อแปลงผล ROM lookup กลับสู่ "frame" ของระบบ เพื่อหลีกเลี่ยงผลลบ วงจรจะแปลง A − B เป็น A + (26 − B) ก่อนแล้วค่อย mod 26

```
flow: A − B + 26 = A + (26−B) → ถ้า ≥ 26 → ลบ 26 → D[6]
```

### Subtractor แรก (6-bit)
**หน้าที่:** คำนวณ C = 26 - B เพื่อแปลงการลบเป็นการบวก (หลีกเลี่ยง negative)

| Constant A (26) | B | C = 26 - B | ความหมาย |
|----------------|---|-----------|---------|
| 26 | 1 | 25 | เตรียมบวกกับ A |
| 26 | 0 | 26 | B=0 → C=26 |
| 26 | 25 | 1 | B=25 → C=1 |

---

### Bit Extender (×2)
**หน้าที่:** zero-extend A และ C เป็น 6-bit ก่อนเข้า Adder เพื่อป้องกัน overflow

| input (6-bit) | Bit Extender output | ป้องกันอะไร |
|--------------|-------------------|-----------|
| 25 (011001) | 011001 | MSB=0 → ไม่ติดลบ |
| 0 (000000) | 000000 | ค่าเป็น 0 ปลอดภัย |
| 26 (011010) | 011010 | ยังพอดี 6-bit |

---

### Adder (6-bit)
**หน้าที่:** บวก A + C = A + (26 - B) = A - B + 26

| A | C = 26-B | A + C | ผล |
|---|---------|------|-----|
| 17 | 25 (26-1) | 42 | 42 ≥ 26 → ลบ 26 → 16 |
| 0 | 25 (26-1) | 25 | 25 < 26 → ใช้เลย |
| 0 | 1 (26-25) | 1 | 1 < 26 → ใช้เลย |

---

### Comparator, Constant, Subtractor สอง, MUX
**หน้าที่:** เหมือน mod26_add ทุกประการ — ตรวจสอบว่าผล ≥ 26 หรือไม่ แล้วลบ 26

| A + C | Comparator A≥B | MUX output (D) | ตัวอย่าง |
|-------|---------------|---------------|--------|
| 42 | 1 | 42 - 26 = 16 | (17 - 1 + 26) mod 26 = 16 ✓ |
| 25 | 0 | 25 | (0 - 1 + 26) mod 26 = 25 ✓ |
| 26 | 1 | 0 | (0 - 0 + 26) mod 26 = 0 ✓ |

---

## Circuit 3 — `plugboard`

**หน้าที่โดยรวม:** สลับตัวอักษรตามสายเชื่อมต่อที่กำหนดไว้ล่วงหน้า — เป็น stage แรกและ stage สุดท้ายของ cipher path สัญญาณผ่านสองครั้ง (ขาไปและขากลับ) โดยใช้ ROM ตัวเดียวกัน เนื่องจาก mapping เป็น symmetric (A↔B หมายความว่า B↔A) ทำให้ plugboard ไม่เปลี่ยนพฤติกรรม encode/decode

```
flow: char_in → [ROM lookup] → char_out
      ตัวที่เสียบสาย: A↔V, H↔Z, ...
      ตัวที่ไม่เสียบ: pass-through
```

### ROM (addrWidth=6, dataWidth=6)
**หน้าที่:** lookup table ขนาด 64 entry — รับ char_in เป็น address คืนตัวอักษรที่ swap แล้ว

| char_in (addr) | ROM data | char_out | ความหมาย |
|---------------|---------|---------|---------|
| 0 (A) | 21 | 21 (V) | A↔V swap |
| 7 (H) | 25 | 25 (Z) | H↔Z swap |
| 4 (E) | 4 | 4 (E) | pass-through (ไม่เสียบสาย) |

---

## Circuit 4 — `reflector`

**หน้าที่โดยรวม:** สะท้อนสัญญาณกลับทิศเพื่อให้สัญญาณวิ่งย้อนผ่านโรเตอร์ทั้งสามอีกครั้ง — เป็น stage กลางของ cipher path (Stage 5) คุณสมบัติที่สำคัญคือ self-inverse: ref[ref[x]] = x เสมอ และ ref[x] ≠ x เสมอ ทำให้ encode = decode และไม่มีตัวอักษรใดเข้ารหัสเป็นตัวเอง

```
flow: char_in → [ROM UKW-B] → char_out
      ref[ref[x]] = x  (self-inverse)
      ref[x] ≠ x       (no fixed point)
```

### ROM (addrWidth=6, dataWidth=6, self-inverse)
**หน้าที่:** lookup table UKW-B — คืนตัวอักษรคู่ที่สะท้อนกลับ โดยมีคุณสมบัติ ref[ref[x]] = x

| char_in (addr) | ROM data | char_out | verify self-inverse |
|---------------|---------|---------|---------------------|
| 0 (A) | 24 | 24 (Y) | ref[24] = 0 = A ✓ |
| 23 (X) | 9 | 9 (J) | ref[9] = 23 = X ✓ |
| 8 (I) | 15 | 15 (P) | ref[15] = 8 = I ✓ |

---

## Circuit 5 — `rotor_fw`

**หน้าที่โดยรวม:** ผ่านสัญญาณผ่านโรเตอร์ในทิศขาไป (forward) — บวก offset ของ pos เพื่อหมุน input ให้อยู่ใน frame ของโรเตอร์ ค้นหาใน ROM, แล้วลบ offset กลับ โรเตอร์ที่เลือก (I, II, หรือ III) ถูกกำหนดด้วย sel pin ทำให้ subcircuit ตัวเดียวรองรับโรเตอร์ทั้ง 3 ตัว

```
flow: char_in + pos → idx (mod26_add)
      idx → ROM[sel] → sub
      sub - pos → char_out (mod26_sub)
```

### Adder (mod26_add ขาแรก)
**หน้าที่:** คำนวณ idx = (char_in + pos) mod 26 — หมุน input ให้อยู่ใน "frame" ของโรเตอร์

| char_in | pos | idx = (char_in + pos) mod 26 | ความหมาย |
|---------|-----|---------------------------|---------|
| 7 (H) | 1 | 8 | มองเข้า rotor ที่ช่อง 8 |
| 25 (Z) | 1 | 0 | wrap รอบ |
| 0 (A) | 0 | 0 | ไม่มี offset |

---

### ROM (×3, addrWidth=5, dataWidth=5)
**หน้าที่:** เก็บ wiring ภายในโรเตอร์แต่ละตัว — idx เข้า → sub ออก (fixed substitution)

| ROM | idx | sub (ROM[idx]) | โรเตอร์ |
|-----|-----|---------------|--------|
| ROM_III | 8 | 17 | Rotor III: ช่อง 8 → ออก 17 |
| ROM_II | 16 | 16 | Rotor II: ช่อง 16 → ออก 16 |
| ROM_I | 16 | 23 | Rotor I: ช่อง 16 → ออก 23 |

---

### Multiplexer (4:1, width=5, sel=2-bit)
**หน้าที่:** เลือก ROM ที่จะใช้ตาม sel — กำหนดว่าใช้โรเตอร์ตัวไหน

| sel | ROM ที่เลือก | ใช้ใน Stage |
|-----|-----------|-----------|
| 0 (00) | ROM_I | Stage 4 (Rotor I fw) |
| 1 (01) | ROM_II | Stage 3 (Rotor II fw) |
| 2 (10) | ROM_III | Stage 2 (Rotor III fw) |

---

### Splitter (6→5-bit สำหรับ ROM address)
**หน้าที่:** ตัด bit5 (MSB) ของ idx ออก เพราะ ROM มี addrWidth=5 (0–31) แต่ idx จาก mod26_add เป็น 6-bit

| idx (6-bit) | Splitter fanout 0 (5-bit) | bit5 (ทิ้ง) | เหตุผล |
|------------|--------------------------|-----------|-------|
| 001000 (8) | 01000 (8) | 0 | bit5=0 เสมอสำหรับ 0–25 |
| 011001 (25) | 11001 (25) | 0 | ยังอยู่ใน 5-bit range |
| 000000 (0) | 00000 (0) | 0 | ปลอดภัย |

---

### Bit Extender (5→6, สำหรับ sub)
**หน้าที่:** ขยาย sub จาก ROM (5-bit) กลับเป็น 6-bit ก่อนเข้า mod26_sub

| ROM output (5-bit) | Bit Extender output (6-bit) | ใช้ทำอะไร |
|-------------------|---------------------------|---------|
| 10001 (17) | 010001 (17) | ส่งเข้า mod26_sub เป็น A |
| 10000 (16) | 010000 (16) | ส่งเข้า mod26_sub เป็น A |
| 10111 (23) | 010111 (23) | ส่งเข้า mod26_sub เป็น A |

---

### Subtractor + MUX (mod26_sub ขาสอง)
**หน้าที่:** คำนวณ char_out = (sub - pos + 26) mod 26 — หมุน output กลับสู่ "frame" ของระบบ

| sub | pos | (sub - pos + 26) mod 26 | char_out |
|-----|-----|------------------------|---------|
| 17 | 1 | 16 | Q(16) ← Stage 2 H encoding |
| 16 | 0 | 16 | Q(16) ← Stage 3 |
| 23 | 0 | 23 | X(23) ← Stage 4 |

---

## Circuit 6 — `rotor_bw`

**หน้าที่โดยรวม:** ผ่านสัญญาณผ่านโรเตอร์ในทิศขากลับ (backward) — โครงสร้างวงจรเหมือน rotor_fw ทุกอย่าง แต่ใช้ Inverse LUT แทน Forward LUT เพื่อ "ย้อนรอย" การ substitution ที่เกิดใน rotor_fw คุณสมบัติ: bw(fw(x, pos), pos) = x เสมอ ทำให้ encode = decode เมื่อ pos เดียวกัน

```
flow: char_in + pos → idx (mod26_add)
      idx → INV_ROM[sel] → sub
      sub - pos → char_out (mod26_sub)

verify: bw(fw(x)) = x ✓
```

### ROM (×3, Inverse LUT)
**หน้าที่:** เก็บ inverse wiring ของโรเตอร์ — ถ้า fw[x]=y แล้ว bw[y]=x

| ROM | idx | INV[idx] | verify กับ fw |
|-----|-----|---------|-------------|
| INV_I | 16 | 7 | fw_I[7]=16 ✓ |
| INV_II | 25 | 18 | fw_II[18]=25 ✓ |
| INV_III | 19 | 9 | fw_III[9]=19 ✓ |

> หลักการ bw(fw(x)) = x ทำให้ตรวจสอบได้เสมอว่า inverse LUT ถูกต้อง

---

## Circuit 7 — `stepping`

**หน้าที่โดยรวม:** ควบคุมตำแหน่ง (pos) ของโรเตอร์ทั้ง 3 ตัว — เป็น sequential logic ส่วนเดียวในระบบที่ทำงานตาม clock โรเตอร์จะหมุนก่อนที่สัญญาณจะวิ่งผ่าน cipher path ทุกครั้ง ทำให้ค่าที่ encode ซ้ำไม่ได้ผลลัพธ์เดิม วงจรนี้ยังรองรับ Double-Stepping ซึ่งเป็น quirk ของเครื่อง Enigma ของจริง

```
Rotor III (pos1): step ทุก keypress
Rotor II  (pos2): step เมื่อ pos1=V หรือ pos2=M (double-step)
Rotor I   (pos3): step เมื่อ pos2=M
```

### Counter (CTR DIV0x19, ×3)
**หน้าที่:** นับ keypress สำหรับแต่ละโรเตอร์ (Rotor III, II, I) โดยนับ 0–25 แล้ว reset

| EN | CLK (rising) | CLR | ผล |
|----|-------------|-----|----|
| 1 | ✓ | 0 | count+1 |
| 0 | ✓ | 0 | คงเดิม |
| X | X | 1 | count=0 ทันที |

---

### NOT Gate
**หน้าที่:** แปลง RST pin (active-high จากผู้ใช้) เป็น CLR สัญญาณสำหรับ Counter

| RST (user) | NOT output | CLR Counter | สถานะ |
|-----------|-----------|------------|-------|
| 0 | 1 | 1 | รีเซ็ต pos=0 |
| 1 | 0 | 0 | นับปกติ |
| 0→1 | 1→0 | 1→0 | รีเซ็ตแล้วปล่อยนับ |

---

### Comparator (×2, A=B)
**หน้าที่:** ตรวจสอบว่า pos ถึง notch หรือยัง — trigger โรเตอร์ถัดไป

| pos | notch constant | A=B output | ความหมาย |
|-----|--------------|-----------|---------|
| 20 | 21 (V) | 0 | ยังไม่ถึง notch |
| 21 | 21 (V) | **1** | ถึง notch → trigger |
| 22 | 21 (V) | 0 | ผ่าน notch ไปแล้ว |

---

### Constant (21 และ 12)
**หน้าที่:** กำหนดตำแหน่ง notch ของแต่ละโรเตอร์ตามประวัติศาสตร์

| Constant | โรเตอร์ | notch ชื่อ | trigger เมื่อ |
|---------|--------|----------|------------|
| 21 (V) | Rotor III | V | pos1 = 21 |
| 12 (M) | Rotor II | M | pos2 = 12 |
| - | Rotor I | Y(24) | ไม่ implement (ไม่ค่อยถึง) |

---

### OR Gate
**หน้าที่:** รวมเงื่อนไข 2 อย่างสำหรับ Rotor II — ถ้า Rotor III ถึง notch **หรือ** Rotor II เองถึง notch → step

| rgt_notch (pos1=21) | mid_notch (pos2=12) | OR output | Rotor II EN |
|--------------------|--------------------|-----------|-----------  |
| 0 | 0 | 0 | ไม่ step |
| 1 | 0 | 1 | step (Rotor III ถึง notch) |
| 0 | 1 | 1 | step (Double-step!) |

---

### AND Gate (×2)
**หน้าที่:** gate ด้วย KEYPRESS — โรเตอร์จะ step ได้เฉพาะเมื่อกดปุ่มจริงๆ เท่านั้น

| KEYPRESS | notch condition | AND → EN | ผล |
|---------|----------------|---------|-----|
| 0 | 1 | 0 | ไม่ step แม้ notch ตรง |
| 1 | 0 | 0 | ไม่ step เพราะยังไม่ถึง notch |
| 1 | 1 | 1 | **step** ✓ |

---

### Bit Extender (5→6, ×3)
**หน้าที่:** ขยาย output ของ Counter (5-bit) เป็น 6-bit เพื่อให้ตรงกับ bus ระบบ

| Counter (5-bit) | Bit Extender (6-bit) | ค่า | signed issue? |
|----------------|---------------------|-----|--------------|
| 10000 (16) | 010000 (16) | 16 | ไม่มี MSB=0 ✓ |
| 11001 (25) | 011001 (25) | 25 | ไม่มี MSB=0 ✓ |
| 11001 (25) ไม่ extend | 111001 | -7 signed | ✗ signed error |

---

## Circuit 8 — `cipher_path`

**หน้าที่โดยรวม:** เป็น datapath combinational ทั้งหมดที่เชื่อม subcircuit 9 ตัวเข้าด้วยกัน — รับ char_in และ pos1/2/3 แล้วส่งสัญญาณผ่าน plugboard → Rotor III → Rotor II → Rotor I → Reflector → Rotor I → Rotor II → Rotor III → plugboard ไม่มี register ภายใน ค่าเปลี่ยนทันทีที่ input เปลี่ยน คุณสมบัติสำคัญ: cipher(cipher(x, pos), pos) = x เสมอ และ cipher(x, pos) ≠ x เสมอ

```
flow: char_in → plug → R3fw → R2fw → R1fw → ref → R1bw → R2bw → R3bw → plug → char_out
pos1 ────────────────────────────────────────────────────────────────────┘
pos2 ─────────────────────────────────────────────────────────────┘
pos3 ──────────────────────────────────────────────────────────────────────┘
```

cipher_path เป็น **combinational pipeline** ไม่มี logic gate ของตัวเอง — ทุก logic อยู่ใน subcircuit

### Subcircuit connections (9 Stage)
**หน้าที่:** ต่อสาย char_in ให้ไหลผ่านทุก stage โดยใช้ pos bus ร่วมกัน

| Stage | Subcircuit | Input | Pos ที่ใช้ | Output |
|-------|-----------|-------|----------|--------|
| 1 | plugboard | char_in | — | after_plug |
| 2 | rotor_fw (sel=2) | after_plug | **pos1** | after_R3_fw |
| 3 | rotor_fw (sel=1) | after_R3_fw | pos2 | after_R2_fw |
| 4 | rotor_fw (sel=0) | after_R2_fw | pos3 | after_R1_fw |
| 5 | reflector | after_R1_fw | — | after_ref |
| 6 | rotor_bw (sel=0) | after_ref | pos3 | after_R1_bw |
| 7 | rotor_bw (sel=1) | after_R1_bw | pos2 | after_R2_bw |
| 8 | rotor_bw (sel=2) | after_R2_bw | **pos1** | after_R3_bw |
| 9 | plugboard | after_R3_bw | — | char_out |

### pos bus (shared)
**หน้าที่:** pos1 ต่อพร้อมกัน Stage 2 และ Stage 8 — ใช้ค่าเดียวกันทั้งขาไปและขากลับ

| pos | Stage ขาไป | Stage ขากลับ | เหตุผล |
|-----|-----------|------------|-------|
| pos1 | Stage 2 (R3 fw) | Stage 8 (R3 bw) | offset ต้องยกเลิกกันพอดี |
| pos2 | Stage 3 (R2 fw) | Stage 7 (R2 bw) | เหมือนกัน |
| pos3 | Stage 4 (R1 fw) | Stage 6 (R1 bw) | เหมือนกัน |

---

## Circuit 9 — `enigma_top`

**หน้าที่โดยรวม:** วงจรระดับสูงสุดที่รวม stepping และ cipher_path เข้าด้วยกัน — รับ char_in (6-bit binary, 0–25) และ KEYPRESS จากผู้ใช้ ให้ stepping หมุนโรเตอร์ก่อน จากนั้น cipher_path คำนวณ output แล้ว Register latch ผล เพิ่ม valid_out เพื่อบอกว่า output พร้อมอ่าน แสดงผล char_out เป็นค่า binary/hex โดยตรงบน Probe หรือ Hex Digit Display

```
flow:
  KEYPRESS → stepping (pos update) → cipher_path → out_reg → char_out [6-bit binary/hex]
                                          ↑
                                       char_in
```

### stepping subcircuit
**หน้าที่:** จัดการตำแหน่ง pos1/2/3 ก่อนที่สัญญาณจะวิ่ง — step ก่อน encode เสมอ

| KEYPRESS | ผลของ stepping | ผลต่อ cipher |
|---------|--------------|------------|
| 1 ครั้ง | pos1+1 | cipher ใช้ pos ใหม่ |
| ต่อเนื่อง 22 ครั้ง | pos2+1 เพิ่มด้วย | output pattern เปลี่ยน |
| RST | pos1=pos2=pos3=0 | เริ่มต้นใหม่ |

---

### cipher_path subcircuit
**หน้าที่:** รับ char_in และ pos → คำนวณ 9 stage → คืน char_out (combinational)

| char_in | pos1 | pos2 | pos3 | char_out |
|---------|------|------|------|---------|
| 7 (H) | 1 | 0 | 0 | 5 (F) |
| 4 (E) | 2 | 0 | 0 | 3 (D) |
| X (ใดก็ได้) | same | same | same | cipher(X) ≠ X เสมอ |

---

### Register out_reg (6-bit)
**หน้าที่:** latch char_out ไว้ 1 cycle — ทำให้อ่านค่าได้หลังจาก KEYPRESS เสร็จ

| D (cipher_path out) | EN (KEYPRESS) | CLK rising | Q (out_reg) |
|--------------------|--------------|-----------|------------|
| 5 (F) | 1 | ✓ | **5 (F)** ← latch |
| 3 (D) | 0 | ✓ | 5 (F) ← ไม่เปลี่ยน |
| 9 (J) | 1 | ✓ | **9 (J)** ← latch ใหม่ |

---

### Register valid_reg (1-bit)
**หน้าที่:** บอกว่า char_out พร้อมอ่านหรือยัง — HIGH เพียง 1 cycle หลัง KEYPRESS

| D (KEYPRESS) | CLK rising | Q (valid_out) | ความหมาย |
|-------------|-----------|--------------|---------|
| 1 | ✓ | 1 | char_out พร้อม |
| 0 | ✓ | 0 | รอ KEYPRESS ถัดไป |
| 1 → 0 | ✓ | 0 | หมดอายุ 1 cycle |

---

### Probe / Hex Digit Display
**หน้าที่:** แสดงค่า char_out (0–25) โดยตรงในรูปแบบ binary หรือ hex ไม่มี display_decoder แล้ว

| char_out (6-bit) | binary | hex | ตัวอักษร |
|----------------|--------|-----|---------|
| 5  | 000101 | 05 | f |
| 3  | 000011 | 03 | d |
| 9  | 001001 | 09 | j |
| 16 | 010000 | 10 | q |

> อ่านค่า binary เทียบกับตารางในไฟล์ README.md (index 0–25 → a–z)
