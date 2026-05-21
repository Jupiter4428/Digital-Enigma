# สคริปอธิบาย Digital Mechanism
## ตัวอย่าง: เข้ารหัสตัวอักษร A

---

> **ตัวอย่าง:** กด `a` (index 0), pos เริ่มต้น = 0,0,0  
> Plugboard: AV BS CG DL FU HZ IN KM OW RX  
> ผลที่จะได้: **A → X**

---

### เริ่มต้น — ก่อนสัญญาณวิ่ง

ก่อนอื่น วงจร **Stepping** ทำงานก่อนเสมอ เพราะในเครื่อง Enigma จริง กลไกเฟืองหมุนโรเตอร์ก่อนที่กระแสไฟจะวิ่ง

```
pos1 (Rotor III): 0 → 1   ← หมุนทุกครั้ง
pos2 (Rotor II):  0 → 0   ← ยังไม่ถึง notch
pos3 (Rotor I):   0 → 0   ← ยังไม่ถึง notch
```

ในวงจรดิจิทัล Stepping คือ Sequential circuit เดียวในระบบ — Counter ทั้ง 3 ตัวนับขึ้นตาม rising edge ของ CLK เมื่อ KEYPRESS=1

---

### Stage 1 — Plugboard (ขาไป)

**Combinational circuit — ROM lookup**

```
char_in = A = index 0
ROM[0] = 21 = V   ← A↔V เสียบสายไว้
char_out = V = index 21
```

วงจรนี้คือ ROM ขนาด 64×6 รับ index เป็น address คืน index ที่ swap แล้ว
ถ้าไม่เสียบสาย ROM[x] = x ผ่านตรง

---

### Stage 2 — Rotor III (ขาไป, pos=1)

**Combinational circuit — mod26_add + ROM + mod26_sub**

```
ขั้น 1: idx = (char_in + pos) mod 26
        idx = (21 + 1) mod 26 = 22
        [บวก pos เพื่อหมุน input ให้อยู่ใน "frame" ของโรเตอร์]

ขั้น 2: sub = ROM_III[22] = 20
        [ค้นหาสายไฟภายในโรเตอร์ที่ตำแหน่ง 22]

ขั้น 3: char_out = (sub - pos + 26) mod 26
        char_out = (20 - 1 + 26) mod 26 = 19 = T
        [ลบ pos กลับ เพื่อแปลงผลกลับสู่ frame ของระบบ]
```

---

### Stage 3 — Rotor II (ขาไป, pos=0)

```
idx      = (19 + 0) mod 26 = 19
sub      = ROM_II[19] = 13 = N
char_out = (13 - 0 + 26) mod 26 = 13 = N
```

pos=0 ทำให้ idx = char_in โดยตรง แต่ wiring ภายใน ROM ยังคง map ค่าใหม่

---

### Stage 4 — Rotor I (ขาไป, pos=0)

```
idx      = (13 + 0) mod 26 = 13
sub      = ROM_I[13] = 22 = W
char_out = (22 - 0 + 26) mod 26 = 22 = W
```

---

### Stage 5 — Reflector

**Combinational circuit — ROM self-inverse**

```
char_in  = W = index 22
ref[22]  = 21 = V
char_out = V = index 21
```

Reflector จับคู่ 13 คู่แบบ symmetric — ref[ref[x]] = x เสมอ
นี่คือจุดที่สัญญาณเปลี่ยนทิศ เพื่อวิ่งย้อนกลับผ่านโรเตอร์

---

### Stage 6 — Rotor I (ขากลับ, pos=0)

**ใช้ Inverse LUT แทน Forward LUT**

```
idx       = (21 + 0) mod 26 = 21
sub       = INV_I[21] = 8 = I
char_out  = (8 - 0 + 26) mod 26 = 8 = I
```

ขากลับใช้ pos เดิมกับขาไป → offset ยกเลิกกันพอดี → bw(fw(x)) = x

---

### Stage 7 — Rotor II (ขากลับ, pos=0)

```
idx       = (8 + 0) mod 26 = 8
sub       = INV_II[8] = 5 = F
char_out  = (5 - 0 + 26) mod 26 = 5 = F
```

---

### Stage 8 — Rotor III (ขากลับ, pos=1)

```
idx       = (5 + 1) mod 26 = 6
sub       = INV_III[6] = 18 = S
char_out  = (18 - 1 + 26) mod 26 = 17 = R
```

---

### Stage 9 — Plugboard (ขากลับ)

**ROM ตัวเดียวกับ Stage 1 — symmetric ในตัว**

```
char_in  = R = index 17
ROM[17]  = 23 = X   ← R↔X เสียบสายไว้
char_out = X = index 23
```

---

### สรุป A → X

```
Stepping:     pos1: 0→1, pos2: 0, pos3: 0

Stage 1  plug_fw  : A(0)  → V(21)   [A↔V swap]
Stage 2  R3_fw    : V(21) → T(19)   [idx=22, ROM=20, 20-1=19]
Stage 3  R2_fw    : T(19) → N(13)   [idx=19, ROM=13]
Stage 4  R1_fw    : N(13) → W(22)   [idx=13, ROM=22]
Stage 5  reflector: W(22) → V(21)   [ref[22]=21]
Stage 6  R1_bw    : V(21) → I(8)    [idx=21, INV=8]
Stage 7  R2_bw    : I(8)  → F(5)    [idx=8,  INV=5]
Stage 8  R3_bw    : F(5)  → R(17)   [idx=6,  INV=18, 18-1=17]
Stage 9  plug_bw  : R(17) → X(23)   [R↔X swap]

A → X  ✓
```

---

### ทำไมถึงพิเศษ?

1. **กดซ้ำได้ผลต่าง** — ครั้งที่ 2 กด A จะ pos1=2 → เส้นทางทุก stage เปลี่ยน → ไม่ได้ X อีกต่อไป

2. **No fixed point** — A ไม่มีทางเข้ารหัสเป็น A เพราะ Reflector ไม่มีตัวอักษรใด map กับตัวเอง

3. **Encode = Decode** — กด X ด้วย pos เดิม จะได้ A กลับมา เพราะทุกวงจรเป็น symmetric
