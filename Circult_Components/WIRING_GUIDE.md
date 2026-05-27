# Enigma Machine — Logisim Build & Test Guide
## สร้างและทดสอบจากวงจรเล็กไปใหญ่ (Bottom-Up)

> ใช้กับ **Logisim-evolution v4.1.0**  
> เปิดไฟล์ `enigma_all.circ` แล้วสร้าง/ทดสอบตาม guide นี้

---

## หลักการ 6-bit

สัญญาณระหว่างวงจรทุกเส้นใช้ **6-bit** เพื่อให้ค่า 0–25 แสดง unsigned เสมอ

```
ค่า 0–25 ใน 6-bit:  0 = 000000,  25 = 011001  (MSB = 0 เสมอ)
```

**การตั้งค่าทั่วไป:**
- ทุก input/output pin ที่รับ/ส่ง char หรือ pos: **width = 6**
- ตั้ง **Radix = Unsigned** บน Probe และ output pin ทุกตัว
- กด **Ctrl+K** เพื่อเปิด simulation ก่อนทดสอบ
- วิธีตั้งค่า component: คลิกขวา → **Edit Properties**

---

| index | ตัว  | binary (6-bit)|
|-------|-----|---------------|
|  0    |  a  |   00 0000     |
|  1    |  b  |   00 0001     |
|  2    |  c  |   00 0010     |
|  3    |  d  |   00 0011     |
|  4    |  e  |   00 0100     |
|  5    |  f  |   00 0101     |
|  6    |  g  |   00 0110     |
|  7    |  h  |   00 0111     |
|  8    |  i  |   00 1000     |
|  9    |  j  |   00 1001     |
| 10    |  k  |   00 1010     |
| 11    |  l  |   00 1011     |
| 12    |  m  |   00 1100     |
| 13    |  n  |   00 1101     |
| 14    |  o  |   00 1110     |
| 15    |  p  |   00 1111     |
| 16    |  q  |   01 0000     |
| 17    |  r  |   01 0001     |
| 18    |  s  |   01 0010     |
| 19    |  t  |   01 0011     |
| 20    |  u  |   01 0100     |
| 21    |  v  |   01 0101     |
| 22    |  w  |   01 0110     |
| 23    |  x  |   01 0111     |
| 24    |  y  |   01 1000     |
| 25    |  z  |   01 1001     |

## Circuit 1 — `mod26_add`
**หน้าที่:** S = (A + B) mod 26

### Components

| # | Component | Library | Properties |
|---|---|---|---|
| 1 | Pin | Wiring | label=`A`, width=**6**, output=false |
| 2 | Pin | Wiring | label=`B`, width=**6**, output=false |
| 3 | Bit Extender | Wiring | in_width=**6**, out_width=**6**, type=zero |
| 4 | Bit Extender | Wiring | in_width=**6**, out_width=**6**, type=zero |
| 5 | Adder | Arithmetic | width=**6** |
| 6 | Comparator | Arithmetic | width=**6**, mode=**unsigned** |
| 7 | Constant | Wiring | value=**0x1A** (=26), width=6 |
| 8 | Subtractor | Arithmetic | width=**6** |
| 9 | Constant | Wiring | value=**0x1A** (=26), width=6 |
| 10 | Multiplexer | Plexers | width=**6**, select=**1** |
| 11 | Pin | Wiring | label=`S`, width=**6**, output=true |

### Wiring

```
A[6] ──[Bit Ext #3]──► A input ของ Adder #5
B[6] ──[Bit Ext #4]──► B input ของ Adder #5

Adder #5 Sum ──┬──► A input Comparator #6
               ├──► A input Subtractor #8
               └──► input 0 (sel=0) MUX #10

Constant #7 (26) ──► B input Comparator #6
Constant #9 (26) ──► B input Subtractor #8
Subtractor #8 Diff ──► input 1 (sel=1) MUX #10

Comparator #6 A≥B ──► sel MUX #10
MUX #10 output[6] ──► S Pin
```

> **Comparator:** ใช้ output **A≥B** (ขากลาง) ไม่ใช่ A>B  
> **Bit Extender:** in=out=6 เป็น no-op แต่รักษา layout ไว้

### Test

| A  | B  | A+B | S คาด | หมายเหตุ |
|----|----|----|------|---------|
| 0  | 0  | 0  | 0    | |
| 25 | 1  | 0  | 0    | idx ของ H→Rotor III (H→Z, (21+1)%26=0) |
| 19 | 0  | 19 | 19   | idx ของ T→Rotor II |
| 13 | 12 | 25 | 25   | max ก่อน wrap |
| 25 | 1  | 26 | 0    | wrap ✓ |
| 13 | 13 | 26 | 0    | wrap ✓ |
| 25 | 25 | 50 | 24   | max case |
| 18 | 1  | 19 | 19   | ใช้ใน Stage 8 |

**Probe ค่ากลาง (วาง Probe บน Adder output ก่อน MUX):**

| A  | B  | Adder sum | Comparator A≥B | S หลัง MUX |
|----|----|-----------|---------------|-----------|
| 25 | 1  | 26        | 1 (HIGH)      | 0         |
| 13 | 12 | 25        | 0 (LOW)       | 25        |

> ถ้า Comparator=0 แต่ S ผิด → ตรวจสาย MUX input 0  
> ถ้า Comparator=1 แต่ S ผิด → ตรวจสาย Subtractor → MUX input 1

---

## Circuit 2 — `mod26_sub`
**หน้าที่:** D = (A − B + 26) mod 26

### Components

| # | Component | Library | Properties |
|---|---|---|---|
| 1 | Pin | Wiring | label=`A`, width=**6**, output=false |
| 2 | Pin | Wiring | label=`B`, width=**6**, output=false |
| 3 | Constant | Wiring | value=**0x1A** (=26), width=**6** |
| 4 | Subtractor | Arithmetic | width=**6** |
| 5 | Bit Extender | Wiring | in_width=**6**, out_width=**6**, type=zero |
| 6 | Bit Extender | Wiring | in_width=**6**, out_width=**6**, type=zero |
| 7 | Adder | Arithmetic | width=**6** |
| 8 | Comparator | Arithmetic | width=**6**, mode=unsigned |
| 9 | Constant | Wiring | value=**0x1A** (=26), width=6 |
| 10 | Subtractor | Arithmetic | width=**6** |
| 11 | Constant | Wiring | value=**0x1A** (=26), width=6 |
| 12 | Multiplexer | Plexers | width=6, select=1 |
| 13 | Pin | Wiring | label=`D`, width=**6**, output=true |

### Wiring

```
Constant #3 (26) ──► A input Subtractor #4
B[6] ──────────────► B input Subtractor #4
Subtractor #4 Diff → C = (26−B)

A[6] ──[Bit Ext #5]──► A input Adder #7
C    ──[Bit Ext #6]──► B input Adder #7

Adder #7 Sum ──┬──► A input Comparator #8
               ├──► A input Subtractor #10
               └──► input 0 MUX #12
Constant #9  ──► B input Comparator #8
Constant #11 ──► B input Subtractor #10
Subtractor #10 Diff ──► input 1 MUX #12
Comparator #8 A≥B ──► sel MUX #12
MUX #12 output[6] ──► D Pin
```

### Test

**ไม่ wrap (A ≥ B):**

| A  | B  | D คาด | หมายเหตุ |
|----|----|------|---------|
| 0  | 0  | 0    | |
| 1  | 1  | 0    | output Stage 2 (Z→Rotor III bw: (1-1)=0) |
| 0  | 0  | 0    | output Stage 3 (A→Rotor II) |
| 0  | 0  | 0    | output Stage 4 (A→Rotor I) |
| 8  | 0  | 8    | output Stage 6 (I→Rotor I bw) |
| 18 | 1  | 17   | output Stage 8 (S→Rotor III bw: (18-1)=17) |

**wrap (A < B):**

| A  | B  | D คาด | หมายเหตุ |
|----|----|------|---------|
| 0  | 1  | 25   | 0−1+26=25 ✓ |
| 0  | 25 | 1    | 0−25+26=1 ✓ |
| 1  | 5  | 22   | |
| 25 | 25 | 0    | |

**Probe ค่ากลาง (A=9, B=1):**

| Probe จุด | ค่าคาด |
|----------|------|
| 26−B (Subtractor #4) | 25 |
| A+(26−B) (Adder #7)  | 34 |
| Comparator A≥B       | 1 (HIGH, 34≥26) |
| D (หลัง MUX)         | 8 (34−26) |

---

## Circuit 3 — `plugboard`
**หน้าที่:** สลับตัวอักษรตามสาย (AV BS CG DL FU HZ IN KM OW RX — 10 คู่)

### Components

| # | Component | Library | Properties |
|---|---|---|---|
| 1 | Pin | Wiring | label=`char_in`, width=**6**, output=false |
| 2 | ROM | Memory | addrWidth=**6**, dataWidth=**6** |
| 3 | Pin | Wiring | label=`char_out`, width=**6**, output=true |

### ROM Data (คลิกขวา ROM → Edit Contents)

Historical 10-pair setting: **AV BS CG DL FU HZ IN KM OW RX**

```
addr/data: 6 6
15 12 06 0b 04 14 02 19 0d 09 0c 03 0a 08 16 0f
10 17 01 13 05 00 0e 11 18 07
```

| addr | data | ความหมาย | addr | data | ความหมาย |
|------|------|---------|------|------|---------|
| 00 (A) | 15 | A→V | 15 (V) | 00 | V→A |
| 01 (B) | 12 | B→S | 12 (S) | 01 | S→B |
| 02 (C) | 06 | C→G | 06 (G) | 02 | G→C |
| 03 (D) | 0b | D→L | 0b (L) | 03 | L→D |
| 05 (F) | 14 | F→U | 14 (U) | 05 | U→F |
| 07 (H) | 19 | H→Z | 19 (Z) | 07 | Z→H |
| 08 (I) | 0d | I→N | 0d (N) | 08 | N→I |
| 0a (K) | 0c | K→M | 0c (M) | 0a | M→K |
| 0e (O) | 16 | O→W | 16 (W) | 0e | W→O |
| 11 (R) | 17 | R→X | 17 (X) | 11 | X→R |
| ที่เหลือ (E J P Q T Y) | =addr | pass-through |

### Wiring

```
char_in[6] ──► Address input ROM
ROM Data output[6] ──► char_out[6] Pin
```

### Test

**Swap pairs:**

| char_in | char_out คาด | addr | data |
|---------|-------------|------|------|
| 0  (A) | 21 (V) | 00 | 15 |
| 21 (V) | 0  (A) | 15 | 00 |
| 1  (B) | 18 (S) | 01 | 12 |
| 7  (H) | 25 (Z) | 07 | 19 |
| 25 (Z) | 7  (H) | 19 | 07 |
| 8  (I) | 13 (N) | 08 | 0d |
| 10 (K) | 12 (M) | 0a | 0c |
| 14 (O) | 22 (W) | 0e | 16 |
| 17 (R) | 23 (X) | 11 | 17 |
| 5  (F) | 20 (U) | 05 | 14 |

**Pass-through (E J P Q T Y):**

| char_in | char_out คาด |
|---------|-------------|
| 4  (E) | 4  (E) |
| 9  (J) | 9  (J) |
| 15 (P) | 15 (P) |
| 16 (Q) | 16 (Q) |
| 19 (T) | 19 (T) |
| 24 (Y) | 24 (Y) |

**Symmetric — swap(swap(x)) = x:**

| x      | swap(x) | swap(swap(x)) |
|--------|---------|--------------|
| 0  (A) | 21 (V)  | 0  (A) ✓ |
| 7  (H) | 25 (Z)  | 7  (H) ✓ |

---

## Circuit 4 — `reflector`
**หน้าที่:** สะท้อนสัญญาณ UKW-B

### Components

| # | Component | Library | Properties |
|---|---|---|---|
| 1 | Pin | Wiring | label=`char_in`, width=**6**, output=false |
| 2 | ROM | Memory | addrWidth=**6**, dataWidth=**6** |
| 3 | Pin | Wiring | label=`char_out`, width=**6**, output=true |

### ROM Data

```
addr/data: 6 6
18 11 14 07 10 12 0b 03 0f 17 0d 06 0e 0a 0c 08
04 01 05 19 02 16 15 09 00 13
```

> UKW-B: A↔Y B↔R C↔U D↔H E↔Q F↔S G↔L H↔D I↔P J↔X K↔N L↔G M↔O

### Wiring

```
char_in[6] ──► Address input ROM
ROM Data output[6] ──► char_out[6] Pin
```

### Test

**Known pairs:**

| char_in | char_out คาด | addr | data |
|---------|-------------|------|------|
| 0  (A) | 24 (Y) | 00 | 18 |
| 24 (Y) | 0  (A) | 18 | 00 |
| 23 (X) | 9  (J) | 17 | 09 | ← Stage 5 ของ H encoding |
| 9  (J) | 23 (X) | 09 | 17 |
| 8  (I) | 15 (P) | 08 | 0f |

**Self-inverse — ref(ref(x)) = x:**

| x      | ref(x) | ref(ref(x)) |
|--------|--------|------------|
| 0  (A) | 24 (Y) | 0  (A) ✓ |
| 23 (X) | 9  (J) | 23 (X) ✓ |

**No fixed point — ref(x) ≠ x สำหรับทุก x ∈ 0..25**

---

## Circuit 5 — `rotor_fw`
**หน้าที่:** ผ่านโรเตอร์ขาไป  
**Logic:** idx=(char_in+pos)%26 → ROM[sel][idx] → sub → (sub−pos+26)%26

### Components

| # | Component | Library | Properties |
|---|---|---|---|
| 1 | Pin | Wiring | label=`char_in`, width=**6**, output=false |
| 2 | Pin | Wiring | label=`pos`, width=**6**, output=false |
| 3 | Pin | Wiring | label=`sel`, width=**2**, output=false |
| 4 | Adder | Arithmetic | width=**6** (mod26_add) |
| 5 | Comparator | Arithmetic | width=**6**, mode=unsigned |
| 6 | Constant | Wiring | value=**0x1A**, width=6 |
| 7 | Subtractor | Arithmetic | width=**6** |
| 8 | Constant | Wiring | value=**0x1A**, width=6 |
| 9 | Multiplexer | Plexers | width=**6**, select=1 (sum หรือ sum-26) |
| 10 | Splitter | Wiring | incoming=6, fanout0=bits0–4 (5-bit→ROM addr), fanout1=bit5 (ทิ้ง) |
| 11 | ROM Rotor I fw | Memory | addrWidth=**5**, dataWidth=**5** |
| 12 | ROM Rotor II fw | Memory | addrWidth=**5**, dataWidth=**5** |
| 13 | ROM Rotor III fw | Memory | addrWidth=**5**, dataWidth=**5** |
| 14 | Multiplexer | Plexers | width=**5**, select=**2** (4:1 ROM selector) |
| 15 | Bit Extender | Wiring | in_width=**5**, out_width=**6**, type=zero (sub→6-bit) |
| 16 | Bit Extender | Wiring | in_width=**6**, out_width=**6**, type=zero (pos) |
| 17 | Adder | Arithmetic | width=**6** (mod26_sub) |
| 18 | Comparator | Arithmetic | width=**6**, mode=unsigned |
| 19 | Constant | Wiring | value=**0x1A**, width=6 |
| 20 | Subtractor | Arithmetic | width=**6** |
| 21 | Constant | Wiring | value=**0x1A**, width=6 |
| 22 | Multiplexer | Plexers | width=**6**, select=1 |
| 23 | Pin | Wiring | label=`char_out`, width=**6**, output=true |

### ROM Data

**ROM_I_fw (Rotor I: EKMFLGDQVZNTOWYHXUSPAIBRCJ)**
```
addr/data: 5 5
04 0a 0c 05 0b 06 03 10 15 19 0d 13 0e 16 18 07
17 14 12 0f 00 08 01 11 02 09
```

**ROM_II_fw (Rotor II: AJDKSIRUXBLHWTMCQGZNPYFVOE)**
```
addr/data: 5 5
00 09 03 0a 12 08 11 14 17 01 0b 07 16 13 0c 02
10 06 19 0d 0f 18 05 15 0e 04
```

**ROM_III_fw (Rotor III: BDFHJLCPRTXVZNYEIWGAKMUSQO)**
```
addr/data: 5 5
01 03 05 07 09 0b 02 0f 11 13 17 15 19 0d 18 04
08 16 06 00 0a 0c 14 12 10 0e
```

### Wiring

```
── mod26_add (idx = char_in + pos) ──
char_in[6] ──► A input Adder #4
pos[6]     ──► B input Adder #4
Adder #4 Sum ──┬──► Comparator #5 / Subtractor #7 / MUX #9 input 0
Constant #6 ──► B Comparator #5
Constant #8 ──► B Subtractor #7
Subtractor #7 Diff ──► MUX #9 input 1
Comparator #5 A≥B ──► sel MUX #9

── Splitter (6→5-bit สำหรับ ROM address) ──
MUX #9 output[6] ──► Splitter #10
Splitter fanout 0 (bits 0–4, 5-bit) ──► Address ROM_I/II/III #11/#12/#13
Splitter fanout 1 (bit5) ──► ทิ้ง (ไม่ต่อสาย)

── ROM selector ──
ROM_I  output[5] ──► MUX #14 input 1   [sel=01]
ROM_II output[5] ──► MUX #14 input 2   [sel=10]
ROM_III output[5]──► MUX #14 input 3   [sel=11]
MUX #14 input 0 ──► Constant 0x00 (unused)
sel[2] ──► sel MUX #14

── mod26_sub (char_out = sub − pos) ──
MUX #14 output[5] ──► Bit Extender #15 (5→6) ──► A input Adder #17
pos[6]            ──► Bit Extender #16 (6→6) ──► B input Adder #17
Adder #17 Sum ──┬──► Comparator #18 / Subtractor #20 / MUX #22 input 0
Constant #19 ──► B Comparator #18
Constant #21 ──► B Subtractor #20
Subtractor #20 Diff ──► MUX #22 input 1
Comparator #18 A≥B ──► sel MUX #22
MUX #22 output[6] ──► char_out Pin
```

> **sel values:** 01=Rotor I, 10=Rotor II, 11=Rotor III  
> ใน cipher_path: Constant(0x2)→sel ของ Rotor III, (0x1)→Rotor II, (0x0)→Rotor I

### Test

**sel mapping (actual):**

| sel | โรเตอร์ |
|-----|---------|
| 0   | Rotor I |
| 1   | Rotor II |
| 2   | Rotor III |

**Rotor III (sel=2) — Stage 2:**

| char_in | pos | idx | ROM_III[idx] | char_out | คาด |
|---------|-----|-----|------------|---------|-----|
| 7  (H) | 1   | 8   | 17 (11h)   | (17-1+26)%26=16 | 16 (Q) ✓ |
| 0  (A) | 0   | 0   | 1  (01h)   | 1               | 1  (B) |
| 25 (Z) | 1   | 0   | 1  (01h)   | (1-1+26)%26=0   | 0  (A) |

**Rotor II (sel=1) — Stage 3:**

| char_in | pos | idx | ROM_II[idx] | char_out | คาด |
|---------|-----|-----|-----------|---------|-----|
| 16 (Q) | 0   | 16  | 16 (10h)  | 16 | 16 (Q) ✓ |
| 0  (A) | 0   | 0   | 0  (00h)  | 0  | 0  (A) |

**Rotor I (sel=0) — Stage 4:**

| char_in | pos | idx | ROM_I[idx] | char_out | คาด |
|---------|-----|-----|----------|---------|-----|
| 16 (Q) | 0   | 16  | 23 (17h) | 23 | 23 (X) ✓ |
| 0  (A) | 0   | 0   | 4  (04h) | 4  | 4  (E) |

**Probe ค่ากลาง (char_in=7, pos=1, sel=2):**

| Probe จุด | ค่าคาด |
|----------|------|
| Adder #4 output (idx ก่อน Splitter) | 8 |
| ROM_III output (sub) | 17 |
| char_out (หลัง mod26_sub) | 16 |

---

## Circuit 6 — `rotor_bw`
**หน้าที่:** ผ่านโรเตอร์ขากลับ (โครงสร้างเหมือน rotor_fw ทุกอย่าง เปลี่ยนแค่ ROM)

### ROM Data

**ROM_I_bw:**
```
addr/data: 5 5
14 16 18 06 00 03 05 0f 15 19 01 04 02 0a 0c 13
07 17 12 0b 11 08 0d 10 0e 09
```

**ROM_II_bw:**
```
addr/data: 5 5
00 09 0f 02 19 16 11 0b 05 01 03 0a 0e 13 18 14
10 06 04 0d 07 17 0c 08 15 12
```

**ROM_III_bw:**
```
addr/data: 5 5
13 00 06 01 0f 02 12 03 10 04 14 05 15 0d 19 07
18 08 17 09 16 0b 11 0a 0e 0c
```

> Wiring เหมือน rotor_fw ทุกอย่าง เปลี่ยนแค่ ROM contents

### Test

**Stage 6 — Rotor I bw (sel=0):**

| char_in | pos | idx | INV_I[idx] | char_out | คาด |
|---------|-----|-----|-----------|---------|-----|
| 9  (J) | 0   | 9   | 25 (19h)  | 25 | 25 (Z) ✓ |
| 0  (A) | 0   | 0   | 20 (14h)  | 20 | 20 (U) |

**Stage 7 — Rotor II bw (sel=1):**

| char_in | pos | idx | INV_II[idx] | char_out | คาด |
|---------|-----|-----|------------|---------|-----|
| 25 (Z) | 0   | 25  | 18 (12h)   | 18 | 18 (S) ✓ |

**Stage 8 — Rotor III bw (sel=2):**

| char_in | pos | idx | INV_III[idx] | char_out | คาด |
|---------|-----|-----|-------------|---------|-----|
| 18 (S) | 1   | 19  | 9  (09h)    | (9-1+26)%26=8 | 8 (I) ✓ |

**Inverse property — bw(fw(x)) = x (ต้องทำ 2 ขั้น):**

```
ขั้น 1: ป้อน char_in เข้า rotor_fw  → ได้ fw_out
ขั้น 2: ป้อน fw_out เข้า rotor_bw  → ต้องได้ char_in กลับมา
```

| char_in | pos | sel | ขั้น 1 rotor_fw → fw_out | ขั้น 2 rotor_bw(fw_out) | ต้องได้ |
|---------|-----|-----|------------------------|------------------------|---------|
| 7  (H) | 1   | 2   | 16 (Q)                 | 7  (H) ✓               | char_in |
| 0  (A) | 0   | 0   | 4  (E)                 | 0  (A) ✓               | char_in |

> **หมายเหตุ:** `rotor_bw(0, 0, 0) = 20` คือ **ถูก** (INV\_I[0]=20)  
> ต้องใส่ **fw\_out=4** เข้า bw ไม่ใช่ใส่ 0 — เพราะ inverse คือ bw(fw(x)) ไม่ใช่ bw(x)

---

## Circuit 7 — `stepping`
**หน้าที่:** Double-Stepping ควบคุมตำแหน่ง Rotor

### Components

| # | Component | Library | Properties |
|---|---|---|---|
| 1 | Pin | Wiring | label=`CLK`, width=1, output=false |
| 2 | Pin | Wiring | label=`RST`, width=1, output=false |
| 3 | Pin | Wiring | label=`KEYPRESS`, width=1, output=false |
| 4 | NOT Gate | Gates | size=30 |
| 5 | Counter | Memory | width=**5**, max=**25**, label=`Rotor_III` |
| 6 | Counter | Memory | width=**5**, max=**25**, label=`Rotor_II` |
| 7 | Counter | Memory | width=**5**, max=**25**, label=`Rotor_I` |
| 8 | Comparator | Arithmetic | width=5, mode=unsigned |
| 9 | Constant | Wiring | value=**0x15** (=21), width=5 |
| 10 | Comparator | Arithmetic | width=5, mode=unsigned |
| 11 | Constant | Wiring | value=**0x0C** (=12), width=5 |
| 12 | OR Gate | Gates | inputs=2 |
| 13 | AND Gate | Gates | inputs=2 (→ EN Rotor_II) |
| 14 | AND Gate | Gates | inputs=2 (→ EN Rotor_I) |
| 15 | Bit Extender | Wiring | in_width=**5**, out_width=**6**, type=zero |
| 16 | Bit Extender | Wiring | in_width=**5**, out_width=**6**, type=zero |
| 17 | Bit Extender | Wiring | in_width=**5**, out_width=**6**, type=zero |
| 18 | Pin | Wiring | label=`POS1`, width=**6**, output=true |
| 19 | Pin | Wiring | label=`POS2`, width=**6**, output=true |
| 20 | Pin | Wiring | label=`POS3`, width=**6**, output=true |

### Counter Pin Layout

```
Counter (facing east):
  EN  (left-middle) ← ต่อ enable ที่นี่
  CLR (left-bottom) ← ต่อ CLR ที่นี่
  CK▲ (bottom)      ← ต่อ CLK ที่นี่
  Q   (right, 5-bit)← output
```

### Wiring

```
RST ──► NOT Gate ──► CLR ของ Counter #5, #6, #7
CLK ──► CK(▲) ของ Counter #5, #6, #7

KEYPRESS ──► EN ของ Counter_III (#5)

Counter_III Q[5] ──┬──► Comparator #8 A input  [rgt_notch: pos1==21?]
                   └──► Bit Extender #15 (5→6) ──► POS1 Pin

Constant #9 (21) ──► Comparator #8 B input

Counter_II Q[5] ──┬──► Comparator #10 A input  [mid_notch: pos2==12?]
                  └──► Bit Extender #16 (5→6) ──► POS2 Pin

Constant #11 (12) ──► Comparator #10 B input

Counter_I Q[5] ──► Bit Extender #17 (5→6) ──► POS3 Pin

Comparator #8  A=B (rgt_notch) ──► OR Gate #12 input 0
Comparator #10 A=B (mid_notch) ──┬► OR Gate #12 input 1
                                 └► AND Gate #14 input 0 also

OR Gate #12 output ──► AND Gate #13 input 0
KEYPRESS ──────────► AND Gate #13 input 1
KEYPRESS ──────────► AND Gate #14 input 1

AND Gate #13 output ──► EN Counter_II (#6)
AND Gate #14 output ──► EN Counter_I  (#7)
```

> **Comparator:** ใช้ output **A=B** (กลาง)  
> **Counter:** width=5, max=25 — ต่อ Bit Extender 5→6 ก่อน POS output pin

### Test

**วิธี reset และ test (manual clock):**

```
Reset:
  RST → 0        (NOT gate → CLR=1 → counter = 0)
  RST → 1        (NOT gate → CLR=0 → พร้อมนับ)

กด keypress 1 ครั้ง:
  KEYPRESS → 1   (ค้างไว้, EN=1)
  CLK → 0 → 1   (rising edge → counter นับ)
  อ่าน POS1/2/3
  CLK → 0        (พร้อมสำหรับครั้งต่อไป)

หรือใช้ Auto-Tick (Ctrl+K):
  RST → 0 → 1   (reset ก่อน)
  KEYPRESS → 1   (ค้างไว้)
  clock วิ่งอัตโนมัติ → POS เพิ่มทุก cycle
```

**Basic stepping:**

| KEYPRESS กด | POS1 | POS2 | POS3 |
|------------|------|------|------|
| 1           | 1    | 0    | 0    |
| 5           | 5    | 0    | 0    |
| 25          | 25   | 0    | 0    |

**Rollover mod 26:**

| POS1 ก่อน | กด | POS1 หลัง |
|----------|---|----------|
| 25       | 1 | 0 ✓      |

**Notch trigger (Rotor III ถึง V=21):**

| POS1 ก่อน | POS2 | กด | POS1 หลัง | POS2 หลัง | POS3 หลัง |
|----------|------|----|----------|----------|----------|
| 20       | 0    | 1  | 21       | 0        | 0  ← ยังไม่ trigger |
| 21       | 0    | 1  | 22       | **1**    | 0  ← notch! ✓ |

**Double-step (Rotor II ที่ notch M=12):**

| POS1 | POS2 | POS3 | กด | POS1 | POS2   | POS3   |
|------|------|------|----|------|--------|--------|
| 0    | 12   | 0    | 1  | 1    | **13** | **1** ← double-step ✓ |
| 21   | 12   | 0    | 1  | 22   | **13** | **1** ← ทั้งสองกรณีพร้อมกัน ✓ |

---

## Circuit 8 — `cipher_path`
**หน้าที่:** 9-stage cipher datapath (combinational ทั้งหมด)

### Components

| # | Component | Library | Properties |
|---|---|---|---|
| 1 | Pin | Wiring | label=`char_in`, width=**6**, output=false |
| 2 | Pin | Wiring | label=`pos1`, width=**6**, output=false |
| 3 | Pin | Wiring | label=`pos2`, width=**6**, output=false |
| 4 | Pin | Wiring | label=`pos3`, width=**6**, output=false |
| 5 | **plugboard** subcircuit | Base | — Stage 1 |
| 6 | **rotor_fw** subcircuit | Base | — Stage 2, sel=2 (Rotor III) |
| 7 | Constant | Wiring | value=**0x2**, width=2 |
| 8 | **rotor_fw** subcircuit | Base | — Stage 3, sel=1 (Rotor II) |
| 9 | Constant | Wiring | value=**0x1**, width=2 |
| 10 | **rotor_fw** subcircuit | Base | — Stage 4, sel=0 (Rotor I) |
| 11 | Constant | Wiring | value=**0x0**, width=2 |
| 12 | **reflector** subcircuit | Base | — Stage 5 |
| 13 | **rotor_bw** subcircuit | Base | — Stage 6, sel=0 (Rotor I) |
| 14 | Constant | Wiring | value=**0x0**, width=2 |
| 15 | **rotor_bw** subcircuit | Base | — Stage 7, sel=1 (Rotor II) |
| 16 | Constant | Wiring | value=**0x1**, width=2 |
| 17 | **rotor_bw** subcircuit | Base | — Stage 8, sel=2 (Rotor III) |
| 18 | Constant | Wiring | value=**0x2**, width=2 |
| 19 | **plugboard** subcircuit | Base | — Stage 9 |
| 20 | Pin | Wiring | label=`char_out`, width=**6**, output=true |

### Wiring

```
char_in ──► plugboard #5
plugboard #5 char_out ──► rotor_fw #6 char_in
Constant(2) ──► rotor_fw #6 sel  │  pos1 ──► rotor_fw #6 pos

rotor_fw #6 char_out ──► rotor_fw #8 char_in
Constant(1) ──► rotor_fw #8 sel  │  pos2 ──► rotor_fw #8 pos

rotor_fw #8 char_out ──► rotor_fw #10 char_in
Constant(0) ──► rotor_fw #10 sel │  pos3 ──► rotor_fw #10 pos

rotor_fw #10 char_out ──► reflector #12 char_in

reflector #12 char_out ──► rotor_bw #13 char_in
Constant(0) ──► rotor_bw #13 sel │  pos3 ──► rotor_bw #13 pos

rotor_bw #13 char_out ──► rotor_bw #15 char_in
Constant(1) ──► rotor_bw #15 sel │  pos2 ──► rotor_bw #15 pos

rotor_bw #15 char_out ──► rotor_bw #17 char_in
Constant(2) ──► rotor_bw #17 sel │  pos1 ──► rotor_bw #17 pos

rotor_bw #17 char_out ──► plugboard #19 char_in
plugboard #19 char_out ──► char_out Pin
```

> **pos bus:** pos1 ต่อพร้อมกัน rotor_fw#6 (Stage2) และ rotor_bw#17 (Stage8)  
> pos2: rotor_fw#8 และ rotor_bw#15 │ pos3: rotor_fw#10 และ rotor_bw#13

### Test

**H encoding ทีละ stage (pos1=1, pos2=0, pos3=0) — วาง Probe บน output ของแต่ละ subcircuit:**

| Stage | Subcircuit | Probe ที่ | ค่าคาด |
|-------|-----------|---------|--------|
| 1 | plugboard #5  | char_out | 25 (Z) | ← H↔Z swap |
| 2 | rotor_fw #6   | char_out | 0  (A) |
| 3 | rotor_fw #8   | char_out | 0  (A) |
| 4 | rotor_fw #10  | char_out | 4  (E) |
| 5 | reflector #12 | char_out | 16 (Q) |
| 6 | rotor_bw #13  | char_out | 7  (H) |
| 7 | rotor_bw #15  | char_out | 11 (L) |
| 8 | rotor_bw #17  | char_out | 20 (U) |
| 9 | plugboard #19 | char_out | 5  (F) | ← U↔F swap |

> ถ้า Stage ใดผิดครั้งแรก → ปัญหาอยู่ที่วงจรนั้น ย้อนกลับไป Step 3–6

**Known values (HELLO encoding — plugboard AV BS CG DL FU HZ IN KM OW RX):**

| char_in | pos1 | pos2 | pos3 | char_out คาด | หมายเหตุ |
|---------|------|------|------|-------------|---------|
| 7  (H) | 1    | 0    | 0    | 5  (F) | H↔Z swap ขาไป, U↔F swap ขากลับ |
| 4  (E) | 2    | 0    | 0    | 3  (D) | E pass-through ขาไป, L↔D swap ขากลับ |
| 11 (L) | 3    | 0    | 0    | 9  (J) | L↔D swap ขาไป, J pass-through ขากลับ |
| 11 (L) | 4    | 0    | 0    | 3  (D) | L↔D swap ขาไป, L↔D swap ขากลับ |
| 14 (O) | 5    | 0    | 0    | 16 (Q) | O↔W swap ขาไป, Q pass-through ขากลับ |

> HELLO → **FDJDQ**

**Decode (ใส่ cipher กลับด้วย pos เดิม):**

| char_in | pos1 | char_out คาด |
|---------|------|-------------|
| 5  (F) | 1    | 7  (H) ✓ |
| 3  (D) | 2    | 4  (E) ✓ |

**Self-inverse — cipher(cipher(x)) = x:**

| char_in | pos1 | cipher(x) | cipher(cipher(x)) |
|---------|------|-----------|------------------|
| 0  (A) | 0    | 16 (Q)    | 0  (A) ✓ |
| 16 (Q) | 0    | 0  (A)    | 16 (Q) ✓ |

**No fixed point — cipher(x) ≠ x สำหรับทุก x ∈ 0..25 ทุก pos**

---

## Circuit 9 — `enigma_top`
**หน้าที่:** เชื่อมทุกอย่าง + Binary input + 7-Segment output

### Components

| # | Component | Library | Properties |
|---|---|---|---|
| 1 | Pin | Wiring | label=`CLK`, width=1, output=false |
| 2 | Pin | Wiring | label=`RST`, width=1, output=false |
| 3 | Pin | Wiring | label=`KEYPRESS`, width=1, output=false |
| 4 | Pin | Wiring | label=`char_in`, width=**6**, output=false |
| 5 | **stepping** subcircuit | Base | — |
| 6 | **cipher_path** subcircuit | Base | — |
| 7 | Register | Memory | width=**6**, label=`out_reg` |
| 8 | Register | Memory | width=**1**, label=`valid_reg` |
| 9 | Hex Digit Display | I/O | label=`POS1(III)` |
| 10 | Hex Digit Display | I/O | label=`POS2(II)` |
| 11 | Hex Digit Display | I/O | label=`POS3(I)` |
| 12 | Pin | Wiring | label=`char_out`, width=**6**, output=true |
| 13 | Pin | Wiring | label=`valid_out`, width=1, output=true |

### Flow ภาพรวม

```
char_in[6] (binary input 0–25)
      │
  [Enigma cipher]
      │
  char_out[6] → Register → อ่านค่าเป็น binary/hex โดยตรง (0–25)
```

### Wiring

```
── stepping ──
CLK      ──► stepping CLK
RST      ──► stepping RST
KEYPRESS ──► stepping KEYPRESS
stepping POS1/2/3[6] ──► cipher_path pos1/pos2/pos3
                     └──► Hex Digit Display POS1/2/3 (#9/#10/#11)

── cipher_path ──
char_in[6] ──► cipher_path char_in
cipher_path char_out[6] ──► out_reg D input

── out_reg (6-bit) ──
KEYPRESS       ──► out_reg EN
CLK            ──► out_reg CK(▲)
RST (NOT Gate) ──► out_reg CLR

out_reg Q[6] ──► char_out Pin (#12)

── valid_reg (1-bit) ──
KEYPRESS ──► valid_reg D input
CLK      ──► valid_reg CK(▲)
valid_reg Q ──► valid_out Pin (#13)
```

> **การอ่านผล:** char_out เป็นค่า 0–25 ในแบบ binary  
> ตั้ง Radix = **Unsigned** หรือ **Hex** บน Probe เพื่ออ่านง่าย  
> ดูตาราง index → ตัวอักษร ใน README.md

### ขั้นตอนใช้งาน

```
⚠️  RST=0 → NOT Gate → CLR=1 → counter รีเซ็ต
    RST=1 → NOT Gate → CLR=0 → counter พร้อมนับ

1. RST → 0          (reset counter → pos=0,0,0)
2. RST → 1          (พร้อมนับ)
3. ใส่ char_in      (0–25 ในแบบ binary 6-bit)
4. KEYPRESS → 1     (ค้างไว้)
5. CLK → 0 → 1     (rising edge → counter step → cipher คำนวณ)
6. KEYPRESS → 0
7. อ่าน char_out จาก 7-Segment Display เมื่อ valid_out = 1

❌ ห้ามกด KEYPRESS ขณะ RST=0 → pos=0,0,0 ตลอด → ค่าผิด
```

**Encode HELLO:**

| ครั้ง | char_in | bin    | POS1 | POS2 | POS3 | char_out คาด | bin    |
|------|---------|--------|------|------|------|-------------|--------|
| 1    | 7  (H) | 000111 | 1    | 0    | 0    | 5  (F)      | 000101 |
| 2    | 4  (E) | 000100 | 2    | 0    | 0    | 3  (D)      | 000011 |
| 3    | 11 (L) | 001011 | 3    | 0    | 0    | 9  (J)      | 001001 |
| 4    | 11 (L) | 001011 | 4    | 0    | 0    | 3  (D)      | 000011 |
| 5    | 14 (O) | 001110 | 5    | 0    | 0    | 16 (Q)      | 010000 |
  
> HELLO → **FDJDQ**

**Decode FDJDQ (RST → 0 → 1 ก่อน เพื่อ reset pos กลับ 0,0,0):**

| ครั้ง | char_in  | POS1 | char_out คาด |
|------|----------|------|-------------|
| 1    | 5  (F)  | 1    | 7  (H) |
| 2    | 3  (D)  | 2    | 4  (E) |
| 3    | 9  (J)  | 3    | 11 (L) |
| 4    | 3  (D)  | 4    | 11 (L) |
| 5    | 16 (Q)  | 5    | 14 (O) |

**valid_out timing:**

| สัญญาณ | ค่าคาด |
|--------|--------|
| valid_out ขณะ KEYPRESS=0 | 0 |
| valid_out หลัง KEYPRESS 1 cycle | 1 |
| valid_out รอบถัดไป | 0 |

**Double-step (กด KEYPRESS ซ้ำจน POS1=21):**

| POS1 | POS2 | กด | POS1 หลัง | POS2 หลัง | POS3 หลัง |
|------|------|----|----------|----------|----------|
| 21   | 0    | 1  | 22       | **1**    | 0 ← notch trigger ✓ |

---

## สรุปลำดับ Bottom-Up

```
Step 1: mod26_add     ← ตรวจ (A+B) mod 26, wrap, Comparator
Step 2: mod26_sub     ← ตรวจ (A-B+26) mod 26, wrap
Step 3: plugboard     ← ตรวจ swap, pass-through, symmetric
Step 4: reflector     ← ตรวจ self-inverse, no fixed point
Step 5: rotor_fw      ← ตรวจ 3 rotors, idx probe, output
Step 6: rotor_bw      ← ตรวจ 3 rotors, inverse property
Step 7: stepping      ← ตรวจ counter, notch, double-step, rollover
Step 8: cipher_path   ← ตรวจ 9 stage probe, self-inverse, no fixed point
Step 9: enigma_top    ← ตรวจ HELLO encode/decode, valid_out, double-step
```

**ถ้าพบค่าผิดที่ Step ใด → หยุดแก้ Step นั้นก่อน ไม่ต้องทดสอบ Step ถัดไป**

---

## Quick Reference

```
Plugboard 10 คู่: AV BS CG DL FU HZ IN KM OW RX
HELLO → fdjdq  (Decode: fdjdq → hello)

H(7) → F(5)  [pos1=1, pos2=0, pos3=0]

Stages:
  plug_fw=Z(25) → R3_fw=A(0) → R2_fw=A(0) → R1_fw=E(4)
  → ref=Q(16) → R1_bw=H(7) → R2_bw=L(11) → R3_bw=U(20) → plug_bw=F(5)

mod26_add:  (25,1)→0  (21,1)→22  (13,13)→0
mod26_sub:  (1,1)→0   (20,1)→19  (0,1)→25

plugboard:  A→V  V→A  H→Z  Z→H  E→E (pass)
reflector:  W(22)→V(21)  V(21)→W(22)  A(0)→Y(24)
```
