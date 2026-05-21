# Enigma Machine — Logisim-evolution Implementation

จำลอง **Enigma I** (Wehrmacht/Luftwaffe) ใน Logisim-evolution 4.1.0  
ใช้ Rotor III → II → I และ Reflector B (UKW-B) ตามประวัติศาสตร์

---

## Enigma คืออะไร?

Enigma คือเครื่องเข้ารหัสที่ใช้ในสงครามโลกครั้งที่ 2 โดยกองทัพเยอรมัน  
หลักการพื้นฐานคือ **Substitution Cipher** — แปลงตัวอักษรหนึ่งเป็นอีกตัวอักษรหนึ่ง  
สิ่งที่ทำให้ Enigma พิเศษคือ key ที่เปลี่ยนทุกครั้งที่กดปุ่ม ทำให้ตัวอักษรเดียวกันกด 2 ครั้งได้ผลลัพธ์ต่างกัน

---

## มองในมุม Physical Machine

### การตั้งค่าเครื่องก่อนใช้งาน (Key Settings)

ก่อนเริ่มส่งข้อความ ผู้ปฏิบัติการทั้งสองฝ่ายต้องตั้งค่าเครื่องให้เหมือนกันทุกประการตาม **Codebook** ประจำวัน:

**ขั้นที่ 1 — เลือกและใส่โรเตอร์**
```
ดึงโรเตอร์ออกจากกล่อง เลือก 3 ตัวจาก 5 ตัวที่มี
ใส่เข้าช่องตามลำดับที่กำหนด เช่น III (ขวา) II (กลาง) I (ซ้าย)
แต่ละโรเตอร์มีรอยบาก (notch) และ wiring คนละแบบ
```

**ขั้นที่ 2 — ตั้ง Ring Setting (Ringstellung)**
```
แหวนที่ขอบโรเตอร์ปรับได้ 1–26 ขีด
ทำให้ offset ระหว่าง wiring กับ notch เลื่อนไป
เพิ่มความซับซ้อนอีกชั้น (ไม่ได้ implement ใน project นี้)
```

**ขั้นที่ 3 — ตั้งตำแหน่งเริ่มต้น (Grundstellung)**
```
หมุนโรเตอร์ทั้ง 3 ตัวจนหน้าต่างแสดงตัวอักษรที่กำหนด เช่น A A A
ตัวอักษรที่หน้าต่าง = ตำแหน่ง pos เริ่มต้นของแต่ละโรเตอร์
```

**ขั้นที่ 4 — เสียบสายไฟหน้าเครื่อง (Stecker)**
```
แผง Plugboard ด้านหน้ามีรู 26 รู (A–Z)
ใช้สายไฟเสียบเชื่อมตัวอักษรตามคู่ที่กำหนด เช่น A↔V, H↔Z
ตัวอักษรที่ไม่ได้เสียบสาย = ผ่านตรง (pass-through)

⚠️ ต้องเสียบสายก่อนปิดฝาและเริ่มส่งข้อความ
   ถ้าสายไม่ตรงกัน ถอดรหัสไม่ได้แม้ pos เริ่มต้นถูกต้อง
```

**ใน Logisim (เทียบเท่า):**

| การตั้งค่า Physical | เทียบกับ Logisim |
|--------------------|----------------|
| เลือกโรเตอร์ | sel constant ใน cipher_path (0,1,2) |
| ตั้ง Ring Setting | ไม่ได้ implement |
| ตั้ง Grundstellung (pos เริ่มต้น) | RST → 0 → 1 (reset counter ให้ pos=0) |
| เสียบสาย Plugboard | ROM data ใน Plugboard.circ |

---

เมื่อผู้ปฏิบัติการ **กดปุ่ม** ตัวอักษรหนึ่ง:

### ขั้นที่ 1 — กลไกเฟืองหมุนโรเตอร์ (ก่อนกระแสไฟวิ่ง)

**Physical:** กลไกเชิงกลล้วนๆ เฟืองและสปริงขับให้โรเตอร์หมุนก่อนที่วงจรไฟฟ้าจะทำงาน แต่ละโรเตอร์มีรอยบาก (notch) ที่ขอบ ใช้เป็นจุดที่จะ "carry" ไปยังโรเตอร์ถัดไป

```
Rotor III (ขวา)  : หมุน 1 ขีดทุกครั้งที่กดปุ่ม — เหมือน ones digit ของ odometer
Rotor II  (กลาง) : หมุนเมื่อ Rotor III ผ่าน notch V
                   หรือเมื่อตัวเองอยู่ที่ notch M → Double-stepping
Rotor I   (ซ้าย) : หมุนเมื่อ Rotor II ผ่าน notch M
```

> เหตุผลที่ต้องหมุนก่อน: ถ้าหมุนหลังจากไฟวิ่ง ตัวอักษรเดิมกด 2 ครั้งจะได้ผลเหมือนกัน ซึ่งทำให้ถอดรหัสง่ายขึ้นมาก

**ใน Digital Logic:** `stepping` circuit — Sequential logic ทำงานที่ posedge CLK เมื่อ KEYPRESS=1 Counter ทั้ง 3 ตัว (pos1, pos2, pos3) อัปเดตก่อนที่ cipher_path จะคำนวณ Comparator ตรวจ notch, OR Gate รวม double-step condition, AND Gate กรองด้วย KEYPRESS

---

### ขั้นที่ 2 — กระแสไฟวิ่งจากแป้นพิมพ์ → Plugboard

**Physical:** แผงสายเสียบด้านหน้าเครื่อง (Steckerbrett) มีรู 26 รู ใช้สายเสียบเชื่อมตัวอักษรเป็นคู่ๆ ไฟที่วิ่งเข้ารูของตัวอักษรหนึ่งจะออกมาที่รูของตัวอักษรคู่แทน

```
เสียบสาย H↔Z: กด H → ไฟออกช่อง Z → เข้าโรเตอร์เป็น Z
ไม่เสียบสาย E: กด E → ไฟผ่านตรง → เข้าโรเตอร์เป็น E
```

> Plugboard เพิ่มความซับซ้อนอีกชั้นโดยไม่ขึ้นกับโรเตอร์ — ทำให้จำนวน key ที่เป็นไปได้เพิ่มขึ้นมหาศาล (10 คู่ = ~150 ล้านล้านล้านล้านล้านล้านล้าน combinations)

**ใน Digital Logic:** `plugboard` circuit — ROM lookup ขนาด 64×6 รับ char_in เป็น address คืน char_out ที่ swap แล้ว ถ้าไม่เสียบสาย ROM[x]=x (pass-through)

---

### ขั้นที่ 3 — ไฟวิ่งผ่านโรเตอร์ทั้ง 3 ตัว (ขวา → กลาง → ซ้าย)

**Physical:** แต่ละโรเตอร์คือจานวงกลมที่มีสายไฟ 26 เส้นเดินจากด้านหน้าไปด้านหลังแบบไขว้ สายไฟแต่ละเส้นเชื่อม input ตำแหน่งหนึ่งกับ output อีกตำแหน่ง เนื่องจากโรเตอร์หมุน ช่องไฟที่เข้าจึงไม่ตรงกับตำแหน่ง 0 อีกต่อไป

```
ตัวอย่าง Rotor III หมุนไป 1 ขีด (pos=1):
  กด H(7) → ไฟเข้าช่อง H แต่โรเตอร์หมุน 1 → มองเห็นช่อง H+1=8
  สายไฟช่อง 8 → ออกช่อง 17(R) → หมุนกลับ → ออกจริงที่ 17-1=16(Q)
```

> ยิ่งโรเตอร์หมุนมาก mapping เปลี่ยนทุกครั้ง — ตัวอักษรเดิมกด 2 ครั้งผ่านโรเตอร์คนละ pos จึงได้ผลต่างกัน

**ใน Digital Logic:** `rotor_fw` circuit — mod26_add(char_in, pos) → ROM[idx] → mod26_sub(sub, pos) โดย ROM เก็บ wiring ของโรเตอร์แต่ละตัว, MUX เลือกโรเตอร์ตาม sel

---

### ขั้นที่ 4 — ไฟวิ่งเข้า Reflector (จานซ้ายสุด ไม่หมุน)

**Physical:** จานที่ปลายสุดด้านซ้ายซึ่งไม่มีกลไกหมุน สายไฟ 26 เส้นจับคู่เป็น 13 คู่ ทำให้ไฟที่เข้ามาถูกส่งกลับออกทางอีกด้านหนึ่ง

```
Reflector จับคู่ 13 คู่แบบ symmetric:
  ถ้า A→Y แล้ว Y→A เสมอ
  ไม่มีตัวอักษรใด map กับตัวเอง (ref[x] ≠ x)

ผลที่ได้: สัญญาณวิ่งย้อนกลับผ่านโรเตอร์ชุดเดิม

ทำไม encode = decode:
  กด A → วงจรวิ่ง A → plug → R3 → R2 → R1 → ref → Y
  กด Y (pos เดิม) → วงจรวิ่งย้อนกลับ Y → ref → R1 → R2 → R3 → plug → A
  เพราะทุกชั้นเป็น bijective และ Reflector ทำให้เส้นทางกลับพอดี
```

**ใน Digital Logic:** `reflector` circuit — ROM lookup self-inverse: ROM[ROM[x]] = x เสมอ ทำได้เพราะ ROM ถูกออกแบบให้เป็นคู่ symmetric

---

### ขั้นที่ 5 — ไฟวิ่งย้อนกลับผ่านโรเตอร์ทั้ง 3 (ซ้าย → กลาง → ขวา)

**Physical:** ไฟวิ่งย้อนทิศผ่านโรเตอร์เดิม แต่คราวนี้เข้าจากด้านหลัง ทำให้ใช้ "inverse wiring" — ถ้าขาไป A→B แล้วขากลับจำเป็นต้อง B→A เพื่อให้สมมาตร

```
ขาไป  (Rotor III fw, pos=1): H(7)  → idx=8  → ROM[8]=17  → Q(16)
ขากลับ (Rotor III bw, pos=1): ค่าใดก็ตาม → INV_ROM[idx] → ย้อนกลับ

pos ที่ใช้ขากลับ = pos เดิม → offset ยกเลิกกันพอดี
```

**ใน Digital Logic:** `rotor_bw` circuit — โครงสร้างเหมือน rotor_fw ทุกอย่าง แต่ใช้ Inverse LUT คุณสมบัติ: bw(fw(x, pos), pos) = x

---

### ขั้นที่ 6 — ไฟออกจาก Plugboard อีกครั้ง → หลอดไฟบนแป้นติด

**Physical:** ไฟวิ่งกลับผ่าน Plugboard อีกรอบด้วยสายเดิม (symmetric) แล้วไปจุดหลอดไฟที่อยู่บนแผงตัวอักษรด้านบน ผู้ปฏิบัติการจดตัวอักษรที่หลอดไฟติดเป็น ciphertext

```
ขาไป Plugboard: H(7) → Z(25)   [H↔Z]
ขากลับ Plugboard: ผลจาก rotor chain → ออกเป็นตัวอักษร output
  ตัวอย่าง: U(20) → F(5)        [U↔F]
```

> ใช้ ROM ตัวเดียวกับ Stage 1 เพราะ symmetric: swap(x)→y และ swap(y)→x

**ใน Digital Logic:** `plugboard` subcircuit อีกครั้ง — Stage 9 ใน cipher_path ใช้ subcircuit ตัวเดียวกับ Stage 1 เพราะ ROM เดียวกันทำงาน symmetric ได้เอง

---

| ส่วนประกอบ | ทางกายภาพ | ใน Digital Logic |
|-----------|---------|----------------|
| Plugboard (Steckerbrett) | แผงสายเสียบ 10 คู่ | ROM 64×6 (symmetric lookup) |
| Rotor (Scrambler) | จานวงกลมมีสายไฟ 26 เส้น | ROM LUT + mod26 offset |
| Reflector (Umkehrwalze) | จานสะท้อนปลายสุด ไม่หมุน | ROM 64×6 (self-inverse) |
| กลไกเฟือง | ขับโรเตอร์หมุนก่อนไฟวิ่ง | Counter (Sequential, posedge CLK) |

---

## มองในมุม Digital Logic

Enigma แบ่งเป็น 2 ส่วน:

```
┌──────────────────────────────────────────────────┐
│  SEQUENTIAL LOGIC                                │
│  (Stepping — pos1, pos2, pos3)                  │
│  ทำงานที่ posedge CLK เมื่อ KEYPRESS=1          │
└─────────────────┬────────────────────────────────┘
                  │ pos1[6], pos2[6], pos3[6]
┌─────────────────▼────────────────────────────────┐
│  COMBINATIONAL LOGIC                             │
│  Plugboard → Rotor III → II → I →               │
│  Reflector → Rotor I → II → III → Plugboard     │
│  char_out = f(char_in, pos1, pos2, pos3)         │
└──────────────────────────────────────────────────┘
```

---

## การตั้งค่าเครื่อง

| รายการ | ค่า |
|--------|-----|
| Rotor order | III (ขวา) → II (กลาง) → I (ซ้าย) |
| Reflector | B (UKW-B) |
| Plugboard | **10 คู่:** AV BS CG DL FU HZ IN KM OW RX |
| Bus width | **6-bit** (ค่า 0–25, MSB=0 เสมอ → ไม่มี signed problem) |
| Starting pos | III=0, II=0, I=0 |

### Plugboard 10 คู่ (Historical Setting)

| คู่     |   | คู่     |   | คู่     |
|-----  |---|-----  |---|-------|
| A ↔ V |   | D ↔ L |   | I ↔ N |
| B ↔ S |   | F ↔ U |   | K ↔ M |
| C ↔ G |   | H ↔ Z |   | O ↔ W |
|       |   | R ↔ X |   |       |

Pass-through (ไม่เสียบสาย): **E J P Q T Y**

---

## Architecture

```
char_in [6-bit, 0–25]
     │
     ▼
┌──────────┐
│ Plugboard│  ← ROM 64×6: สลับตัวอักษร 10 คู่
└────┬─────┘
     │
┌────▼─────┐  pos1[6] (Rotor III)
│ Rotor III│  ← rotor_fw + rotor_bw: LUT + offset
└────┬─────┘
┌────▼─────┐  pos2[6] (Rotor II)
│ Rotor II │  ← rotor_fw + rotor_bw
└────┬─────┘
┌────▼─────┐  pos3[6] (Rotor I)
│ Rotor I  │  ← rotor_fw + rotor_bw
└────┬─────┘
┌────▼─────┐
│ Reflector│  ← ROM 64×6: self-inverse (UKW-B)
└────┬─────┘
     │ (สัญญาณย้อนกลับผ่าน Rotor I → II → III → Plugboard)
     ▼
char_out [6-bit, 0–25]
     │
     ▼
┌──────────┐
│ Register │  ← latch ผลที่ posedge CLK เมื่อ KEYPRESS=1
└────┬─────┘
     ▼
  char_out → แสดงผลเป็นเลขฐาน 2 (binary) หรือ hex โดยตรง
```

---

## วงจรทั้งหมด

| วงจร | หน้าที่ | ประเภท |
|------|--------|--------|
| `mod26_add` | S = (A+B) mod 26 | Combinational |
| `mod26_sub` | D = (A−B+26) mod 26 | Combinational |
| `plugboard` | สลับตัวอักษรตามสายเสียบ | Combinational (ROM) |
| `reflector` | สะท้อนสัญญาณ UKW-B | Combinational (ROM) |
| `rotor_fw` | ผ่านโรเตอร์ขาไป | Combinational |
| `rotor_bw` | ผ่านโรเตอร์ขากลับ (Inverse LUT) | Combinational |
| `stepping` | ควบคุม pos ของโรเตอร์ | **Sequential** |
| `cipher_path` | เชื่อม 9 stage combinational | Combinational |
| `enigma_top` | วงจรสมบูรณ์ + input/output | Mixed |

---

## Rotor Stepping

```
Rotor III (pos1): step ทุก keypress
Rotor II  (pos2): step เมื่อ pos1 = V(21)  หรือ pos2 เองที่ M(12) → Double-step
Rotor I   (pos3): step เมื่อ pos2 = M(12)
```

| โรเตอร์ | Notch | step ทุก (เฉลี่ย) |
|---------|-------|----------------|
| Rotor III | V = 21 | 1 keypress |
| Rotor II | M = 12 | ~22 keypress |
| Rotor I | Y = 24 | ~572 keypress |

**Double-Stepping:** เมื่อ Rotor II อยู่ที่ Notch M พอดี Rotor II และ Rotor I จะหมุนพร้อมกันในรอบเดียว

---

## Rotor Wiring

| โรเตอร์ | Wiring |
|---------|--------|
| Rotor I | EKMFLGDQVZNTOWYHXUSPAIBRCJ |
| Rotor II | AJDKSIRUXBLHWTMCQGZNPYFVOE |
| Rotor III | BDFHJLCPRTXVZNYEIWGAKMUSQO |
| Reflector B | YRUHQSLDPXNGOKMIEBFZCWVJAT |

**การคำนวณโรเตอร์:**
```
idx      = (char_in + pos) mod 26    ← บวก offset
sub      = ROM[idx]                   ← lookup wiring
char_out = (sub − pos + 26) mod 26   ← ลบ offset กลับ
```

---

## ทำไม Encode = Decode?

เพราะทุกชั้นเป็น **bijective** และ **symmetric**:

| ชั้น | คุณสมบัติ |
|------|---------|
| Plugboard | swap(swap(x)) = x |
| Rotor + Inverse | bw(fw(x, pos), pos) = x |
| Reflector | ref(ref(x)) = x |

ดังนั้น cipher(cipher(x, pos), pos) = x เสมอ — ป้อน ciphertext กลับด้วย pos เดิมจะได้ plaintext

---

## การแสดงผล

`char_out` เป็นค่า **0–25** (6-bit binary) แสดงตรงๆ บน Probe หรือ Hex Digit Display

```
ตัวอย่าง: char_out = 5 (F)
  Binary:  000101
  Hex:     05
  ดูตาราง: index 5 = f
```

| index | ตัว | hex | binary (6-bit) |
|-------|-----|-----|----------------|
| 0 | a | 00 | 00 0000 |
| 7 | h | 07 | 00 0111 |
| 16 | q | 10 | 01 0000 |
| 25 | z | 19 | 01 1001 |

---

## ตัวอย่าง: เข้ารหัส HELLO

การตั้งค่า: pos = AAA (0,0,0), Plugboard: AV BS CG DL FU HZ IN KM OW RX

| ครั้ง | input | pos III | pos II | pos I | output | หมายเหตุ |
|------|-------|---------|--------|-------|--------|---------|
| 1 | h (7)  | 1 | 0 | 0 | f (5)  | H↔Z ขาไป, U↔F ขากลับ |
| 2 | e (4)  | 2 | 0 | 0 | d (3)  | E pass-through, L↔D ขากลับ |
| 3 | l (11) | 3 | 0 | 0 | j (9)  | L↔D ขาไป, J pass-through ขากลับ |
| 4 | l (11) | 4 | 0 | 0 | d (3)  | L↔D ทั้งสองทิศ |
| 5 | o (14) | 5 | 0 | 0 | q (16) | O↔W ขาไป, Q pass-through ขากลับ |

**HELLO → fdjdq**

**Decode:** ป้อน `fdjdq` (reset pos ก่อน) → ได้ `hello` กลับ ✓

---

## วิธีใช้งานใน Logisim

```
1. RST → 0          (CLR=1 → reset counter → pos=0,0,0)
2. RST → 1          (CLR=0 → พร้อมนับ)
3. ใส่ char_in      (0–25 แบบ binary 6-bit บน Pin)
4. KEYPRESS → 1     (ค้างไว้)
5. CLK → 0 → 1     (rising edge → rotor steps → cipher คำนวณ)
6. KEYPRESS → 0
7. อ่าน char_out    (0–25 binary/hex บน Probe หรือ Register Q)

⚠️  ต้องกด RST=1 ก่อนกด KEYPRESS เสมอ
    ถ้า RST=0 อยู่ → counter ล็อก → pos=0,0,0 ตลอด → ค่าผิด
```

---

## โครงสร้างไฟล์

```
WIRING_GUIDE.md          — วิธีสร้างและทดสอบวงจรทีละวงจร (Bottom-Up)
H_encoding_example.md    — trace H encoding ทีละ stage ทีละตัวแปร
circuit_logic_guide.md   — หน้าที่ logic component ทุกวงจร พร้อมตัวอย่าง
logisim_test_guide.md    — (deprecated, รวมอยู่ใน WIRING_GUIDE แล้ว)

source_code/
  enigma.v               — Verilog RTL implementation
  enigma_tb.v            — Testbench
  circuits/
    *.c / *.h            — C reference implementation
    test_cipher_path.c   — Unit tests

*.circ                   — Logisim-evolution circuit files
  mod26_add.circ
  mod26_sub.circ
  Plugboard.circ
  Reflector.circ
  complete_fw.circ       — rotor_fw
  complete_bw.circ       — rotor_bw
  stepping.circ
  cipher_path.circ
  display_decoder.circ
  main.circ              — enigma_top
```
<table align="center" style="width: 100%; border-collapse: collapse;">
<tr style="background-color: #f8fafc;">
<th align="center" style="padding: 10px;">บทบาท (Role)</th>
<th align="center" style="padding: 10px;">ชื่อ-นามสกุล</th>
<th align="center" style="padding: 10px;">รหัสนักศึกษา</th>
</tr>
<tr>
<td align="center"><b>Circuit Architect & Verification</b></td>
<td align="center">นายกฤตยชญ์ แก้วกำมา</td>
<td align="center">6710535011</td>
</tr>
<tr>
<td align="center"><b>Technical Lead & Algorithm Design & Testing</b></td>
<td align="center">นายวุฒิศักดิ์ บุญกัน</td>
<td align="center">6710535029</td>
</tr>
<tr>
<td align="center"><b>Circuit Architect & Verification</b></td>
<td align="center">นายภูวรัตน์ นาคจันทึก</td>
<td align="center">6710615201</td>
</tr>
<tr>
<td align="center"><b>Circuit Architect & Verification</b></td>
<td align="center">นายธนกฤต โพธิมาศ</td>
<td align="center">6710625010</td>
</tr>
   <tr>
<td align="center"><b>Circuit Architect & Verification</b></td>
<td align="center">นายสายฟ้า เดชะ</td>
<td align="center"> </td>
</tr>
</table>

<p align="right"><i>Project Version BETA
<p align="right"><i>โปรเจคนี้เป็นเพียงโปรเจคในคอร์สเรียน CN262 Digital Logic Design เท่านั้น ยังคงมีบางฟังก์ชั่นที่ไม่ได้ทำการ Implement na kub!
