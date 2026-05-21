// =============================================================================
//  enigma_machine.v  ─  COMPLETE ENIGMA I RTL IMPLEMENTATION (Verilog-2001)
// =============================================================================
//
//  Model  : Enigma I (Wehrmacht / Luftwaffe)
//  Rotors : III (right) → II (middle) → I (left)
//  Reflect: UKW-B  (Reflector B)
//
//  เส้นทางสัญญาณ (Signal Path) ─ 1 keystroke:
//
//   char_in
//      │
//      ▼
//  ┌─────────┐
//  │Plugboard│  ← ด่าน 1: สลับคู่ตัวอักษร (ขาไป)
//  └────┬────┘
//       │
//  ┌────▼────┐  pos1_next
//  │Rotor III│  ← ด่าน 2: โรเตอร์ขวา (forward)
//  └────┬────┘
//  ┌────▼────┐  pos2_next
//  │ Rotor II│  ← ด่าน 3: โรเตอร์กลาง (forward)
//  └────┬────┘
//  ┌────▼────┐  pos3_next
//  │ Rotor I │  ← ด่าน 4: โรเตอร์ซ้าย (forward)
//  └────┬────┘
//  ┌────▼────┐
//  │Reflector│  ← ด่าน 5: สะท้อนกระแส กลับทิศ
//  └────┬────┘
//  ┌────▼────┐  pos3_next
//  │ Rotor I │  ← ด่าน 6: โรเตอร์ซ้าย (backward)
//  └────┬────┘
//  ┌────▼────┐  pos2_next
//  │ Rotor II│  ← ด่าน 7: โรเตอร์กลาง (backward)
//  └────┬────┘
//  ┌────▼────┐  pos1_next
//  │Rotor III│  ← ด่าน 8: โรเตอร์ขวา (backward)
//  └────┬────┘
//  ┌────▼────┐
//  │Plugboard│  ← ด่าน 9: สลับกลับ (ขากลับ)
//  └────┬────┘
//       │
//      ▼  char_out (registered)
//
//  Timing:
//    • โรเตอร์หมุน (pos_next) ก่อนที่กระแสไฟฟ้าจะวิ่ง
//    • char_out ถูก register ที่ posedge หลัง keypress
//    • valid_out สูง 1 cycle พร้อมกับ char_out
//
// =============================================================================
`timescale 1ns/1ps
`default_nettype none


// ─────────────────────────────────────────────────────────────────────────────
//  MODULE 1 : PLUGBOARD  (Steckerbrett)
//  ─ สลับคู่ตัวอักษรที่แผงด้านหน้า (symmetric swap pairs)
//  ─ แก้ไข case entries เพื่อเปลี่ยนการตั้งค่า
//  ─ ถ้าไม่ได้เสียบสายจะ pass-through (default)
// ─────────────────────────────────────────────────────────────────────────────
module plugboard (
    input  wire [4:0] char_in,
    output reg  [4:0] char_out
);
    always @(*) begin
        case (char_in)
            // ─── ตัวอย่างการเสียบสาย 5 คู่ ────────────────────────
            //     (ต้อง symmetric: ถ้า A↔F ต้องมีทั้งสองทิศทาง)
            5'd0 : char_out = 5'd5;   // A ↔ F
            5'd5 : char_out = 5'd0;
            5'd1 : char_out = 5'd17;  // B ↔ R
            5'd17: char_out = 5'd1;
            5'd2 : char_out = 5'd23;  // C ↔ X
            5'd23: char_out = 5'd2;
            5'd3 : char_out = 5'd22;  // D ↔ W
            5'd22: char_out = 5'd3;
            5'd4 : char_out = 5'd10;  // E ↔ K
            5'd10: char_out = 5'd4;
            // ─── ไม่ได้เสียบสาย: ผ่านไปตามปกติ ───────────────────
            default: char_out = char_in;
        endcase
    end
endmodule


// ─────────────────────────────────────────────────────────────────────────────
//  MODULE 2 : ROTOR
//  ─ ROTOR_TYPE = 1 (Rotor I), 2 (Rotor II), 3 (Rotor III)
//  ─ dir = 0 : forward (ขาไป)
//  ─ dir = 1 : backward (ขากลับหลัง Reflector)
//  ─ pos : ตำแหน่งปัจจุบันของโรเตอร์ (0-25)
//
//  การคำนวณ:
//    idx_in  = (char_in + pos)     % 26   ← บวก offset ของโรเตอร์
//    sub_out = wiring_table[idx_in]        ← ผ่านสายไฟข้างใน
//    char_out = (sub_out - pos + 26) % 26  ← ลบ offset กลับ
// ─────────────────────────────────────────────────────────────────────────────
module rotor #(parameter ROTOR_TYPE = 1) (
    input  wire [4:0] char_in,
    input  wire [4:0] pos,
    input  wire       dir,      // 0=forward, 1=backward
    output reg  [4:0] char_out
);
    reg [4:0] idx_in, sub_out;

    // ── Rotor I  : EKMFLGDQVZNTOWYHXUSPAIBRCJ ───────────────────────────────
    //    Notch อยู่ที่ Y (24) — เมื่อโรเตอร์ขวาถึง Y จะดัน Middle ให้หมุน
    function [4:0] fw1; input [4:0] x;
        case(x)
            5'd0 :fw1=5'd4;  5'd1 :fw1=5'd10; 5'd2 :fw1=5'd12; 5'd3 :fw1=5'd5;
            5'd4 :fw1=5'd11; 5'd5 :fw1=5'd6;  5'd6 :fw1=5'd3;  5'd7 :fw1=5'd16;
            5'd8 :fw1=5'd21; 5'd9 :fw1=5'd25; 5'd10:fw1=5'd13; 5'd11:fw1=5'd19;
            5'd12:fw1=5'd14; 5'd13:fw1=5'd22; 5'd14:fw1=5'd24; 5'd15:fw1=5'd7;
            5'd16:fw1=5'd23; 5'd17:fw1=5'd20; 5'd18:fw1=5'd18; 5'd19:fw1=5'd15;
            5'd20:fw1=5'd0;  5'd21:fw1=5'd8;  5'd22:fw1=5'd1;  5'd23:fw1=5'd17;
            5'd24:fw1=5'd2;  default:fw1=5'd9;   // 25→J
        endcase
    endfunction
    function [4:0] bw1; input [4:0] x;  // Inverse of Rotor I
        case(x)
            5'd0 :bw1=5'd20; 5'd1 :bw1=5'd22; 5'd2 :bw1=5'd24; 5'd3 :bw1=5'd6;
            5'd4 :bw1=5'd0;  5'd5 :bw1=5'd3;  5'd6 :bw1=5'd5;  5'd7 :bw1=5'd15;
            5'd8 :bw1=5'd21; 5'd9 :bw1=5'd25; 5'd10:bw1=5'd1;  5'd11:bw1=5'd4;
            5'd12:bw1=5'd2;  5'd13:bw1=5'd10; 5'd14:bw1=5'd12; 5'd15:bw1=5'd19;
            5'd16:bw1=5'd7;  5'd17:bw1=5'd23; 5'd18:bw1=5'd18; 5'd19:bw1=5'd11;
            5'd20:bw1=5'd17; 5'd21:bw1=5'd8;  5'd22:bw1=5'd13; 5'd23:bw1=5'd16;
            5'd24:bw1=5'd14; default:bw1=5'd9;   // 25→Z
        endcase
    endfunction

    // ── Rotor II : AJDKSIRUXBLHWTMCQGZNPYFVOE ───────────────────────────────
    //    Notch อยู่ที่ M (12)
    function [4:0] fw2; input [4:0] x;
        case(x)
            5'd0 :fw2=5'd0;  5'd1 :fw2=5'd9;  5'd2 :fw2=5'd3;  5'd3 :fw2=5'd10;
            5'd4 :fw2=5'd18; 5'd5 :fw2=5'd8;  5'd6 :fw2=5'd17; 5'd7 :fw2=5'd20;
            5'd8 :fw2=5'd23; 5'd9 :fw2=5'd1;  5'd10:fw2=5'd11; 5'd11:fw2=5'd7;
            5'd12:fw2=5'd22; 5'd13:fw2=5'd19; 5'd14:fw2=5'd12; 5'd15:fw2=5'd2;
            5'd16:fw2=5'd16; 5'd17:fw2=5'd6;  5'd18:fw2=5'd25; 5'd19:fw2=5'd13;
            5'd20:fw2=5'd15; 5'd21:fw2=5'd24; 5'd22:fw2=5'd5;  5'd23:fw2=5'd21;
            5'd24:fw2=5'd14; default:fw2=5'd4;   // 25→E
        endcase
    endfunction
    function [4:0] bw2; input [4:0] x;  // Inverse of Rotor II
        case(x)
            5'd0 :bw2=5'd0;  5'd1 :bw2=5'd9;  5'd2 :bw2=5'd15; 5'd3 :bw2=5'd2;
            5'd4 :bw2=5'd25; 5'd5 :bw2=5'd22; 5'd6 :bw2=5'd17; 5'd7 :bw2=5'd11;
            5'd8 :bw2=5'd5;  5'd9 :bw2=5'd1;  5'd10:bw2=5'd3;  5'd11:bw2=5'd10;
            5'd12:bw2=5'd14; 5'd13:bw2=5'd19; 5'd14:bw2=5'd24; 5'd15:bw2=5'd20;
            5'd16:bw2=5'd16; 5'd17:bw2=5'd6;  5'd18:bw2=5'd4;  5'd19:bw2=5'd13;
            5'd20:bw2=5'd7;  5'd21:bw2=5'd23; 5'd22:bw2=5'd12; 5'd23:bw2=5'd8;
            5'd24:bw2=5'd21; default:bw2=5'd18;  // 25→S
        endcase
    endfunction

    // ── Rotor III: BDFHJLCPRTXVZNYEIWGAKMUSQO ───────────────────────────────
    //    Notch อยู่ที่ V (21)
    function [4:0] fw3; input [4:0] x;
        case(x)
            5'd0 :fw3=5'd1;  5'd1 :fw3=5'd3;  5'd2 :fw3=5'd5;  5'd3 :fw3=5'd7;
            5'd4 :fw3=5'd9;  5'd5 :fw3=5'd11; 5'd6 :fw3=5'd2;  5'd7 :fw3=5'd15;
            5'd8 :fw3=5'd17; 5'd9 :fw3=5'd19; 5'd10:fw3=5'd23; 5'd11:fw3=5'd21;
            5'd12:fw3=5'd25; 5'd13:fw3=5'd13; 5'd14:fw3=5'd24; 5'd15:fw3=5'd4;
            5'd16:fw3=5'd8;  5'd17:fw3=5'd22; 5'd18:fw3=5'd6;  5'd19:fw3=5'd0;
            5'd20:fw3=5'd10; 5'd21:fw3=5'd12; 5'd22:fw3=5'd20; 5'd23:fw3=5'd18;
            5'd24:fw3=5'd16; default:fw3=5'd14;  // 25→O
        endcase
    endfunction
    function [4:0] bw3; input [4:0] x;  // Inverse of Rotor III
        case(x)
            5'd0 :bw3=5'd19; 5'd1 :bw3=5'd0;  5'd2 :bw3=5'd6;  5'd3 :bw3=5'd1;
            5'd4 :bw3=5'd15; 5'd5 :bw3=5'd2;  5'd6 :bw3=5'd18; 5'd7 :bw3=5'd3;
            5'd8 :bw3=5'd16; 5'd9 :bw3=5'd4;  5'd10:bw3=5'd20; 5'd11:bw3=5'd5;
            5'd12:bw3=5'd21; 5'd13:bw3=5'd13; 5'd14:bw3=5'd25; 5'd15:bw3=5'd7;
            5'd16:bw3=5'd24; 5'd17:bw3=5'd8;  5'd18:bw3=5'd23; 5'd19:bw3=5'd9;
            5'd20:bw3=5'd22; 5'd21:bw3=5'd11; 5'd22:bw3=5'd17; 5'd23:bw3=5'd10;
            5'd24:bw3=5'd14; default:bw3=5'd12;  // 25→M
        endcase
    endfunction

    // ── Combinational signal path ────────────────────────────────────────────
    always @(*) begin
        // Step 1: บวก position offset (shift entry into rotor's reference frame)
        idx_in = (char_in + pos >= 5'd26) ? char_in + pos - 5'd26 : char_in + pos;

        // Step 2: ผ่านสายไฟภายในโรเตอร์
        if (!dir) begin          // Forward
            case (ROTOR_TYPE)
                2'd1: sub_out = fw1(idx_in);
                2'd2: sub_out = fw2(idx_in);
                2'd3: sub_out = fw3(idx_in);
                default: sub_out = idx_in;
            endcase
        end else begin           // Backward
            case (ROTOR_TYPE)
                2'd1: sub_out = bw1(idx_in);
                2'd2: sub_out = bw2(idx_in);
                2'd3: sub_out = bw3(idx_in);
                default: sub_out = idx_in;
            endcase
        end

        // Step 3: ลบ position offset กลับ (shift exit from rotor's frame)
        char_out = (sub_out >= pos) ? sub_out - pos : sub_out - pos + 5'd26;
    end
endmodule


// ─────────────────────────────────────────────────────────────────────────────
//  MODULE 3 : REFLECTOR B  (UKW-B)
//  ─ YRUHQSLDPXNGOKMIEBFZCWVJAT
//  ─ Symmetric (self-inverse): ref[ref[x]] = x
//  ─ ไม่มีตัวอักษรใดถูก map กับตัวเอง
// ─────────────────────────────────────────────────────────────────────────────
module reflector (
    input  wire [4:0] char_in,
    output reg  [4:0] char_out
);
    always @(*) begin
        case (char_in)
            5'd0 : char_out = 5'd24; // A ↔ Y
            5'd1 : char_out = 5'd17; // B ↔ R
            5'd2 : char_out = 5'd20; // C ↔ U
            5'd3 : char_out = 5'd7;  // D ↔ H
            5'd4 : char_out = 5'd16; // E ↔ Q
            5'd5 : char_out = 5'd18; // F ↔ S
            5'd6 : char_out = 5'd11; // G ↔ L
            5'd7 : char_out = 5'd3;  // H ↔ D
            5'd8 : char_out = 5'd15; // I ↔ P
            5'd9 : char_out = 5'd23; // J ↔ X
            5'd10: char_out = 5'd13; // K ↔ N
            5'd11: char_out = 5'd6;  // L ↔ G
            5'd12: char_out = 5'd14; // M ↔ O
            5'd13: char_out = 5'd10; // N ↔ K
            5'd14: char_out = 5'd12; // O ↔ M
            5'd15: char_out = 5'd8;  // P ↔ I
            5'd16: char_out = 5'd4;  // Q ↔ E
            5'd17: char_out = 5'd1;  // R ↔ B
            5'd18: char_out = 5'd5;  // S ↔ F
            5'd19: char_out = 5'd25; // T ↔ Z
            5'd20: char_out = 5'd2;  // U ↔ C
            5'd21: char_out = 5'd22; // V ↔ W
            5'd22: char_out = 5'd21; // W ↔ V
            5'd23: char_out = 5'd9;  // X ↔ J
            5'd24: char_out = 5'd0;  // Y ↔ A
            default: char_out = 5'd19; // Z ↔ T
        endcase
    end
endmodule


// ─────────────────────────────────────────────────────────────────────────────
//  MODULE 4 : ENIGMA Main (Integration)
// ─────────────────────────────────────────────────────────────────────────────
//  Ports:
//    clk        : system clock
//    rst_n      : active-low reset (โหลด init positions ใหม่)
//    keypress   : pulse 1 cycle = กดปุ่ม 1 ครั้ง
//    char_in    : ตัวอักษรที่กด (0=A … 25=Z)
//    init_pos1  : ตำแหน่งเริ่มต้น Rotor III (ขวา)
//    init_pos2  : ตำแหน่งเริ่มต้น Rotor II  (กลาง)
//    init_pos3  : ตำแหน่งเริ่มต้น Rotor I   (ซ้าย)
//    char_out   : ผลลัพธ์ (valid 1 cycle หลัง keypress)
//    valid_out  : สูง 1 cycle เมื่อ char_out พร้อม
//    dbg_pos*   : debug: ตำแหน่งโรเตอร์ปัจจุบัน
// ─────────────────────────────────────────────────────────────────────────────
module enigma_machine (
    input  wire       clk,
    input  wire       rst_n,
    input  wire       keypress,
    input  wire [4:0] char_in,
    // ── Starting positions (ตั้งค่าก่อน rst_n ↑) ──
    input  wire [4:0] init_pos1,   // Rotor III (rightmost)
    input  wire [4:0] init_pos2,   // Rotor II  (middle)
    input  wire [4:0] init_pos3,   // Rotor I   (leftmost)
    // ── Outputs ────────────────────────────────────
    output reg  [4:0] char_out,
    output reg        valid_out,
    // ── Debug ports ────────────────────────────────
    output wire [4:0] dbg_pos1,    // Current Rotor III position
    output wire [4:0] dbg_pos2,    // Current Rotor II  position
    output wire [4:0] dbg_pos3     // Current Rotor I   position
);
    // ── Rotor position registers ─────────────────────────────────────────────
    reg [4:0] pos1, pos2, pos3;   // pos1=Rotor III(R), pos2=Rotor II(M), pos3=Rotor I(L)

    assign dbg_pos1 = pos1;
    assign dbg_pos2 = pos2;
    assign dbg_pos3 = pos3;

    // ── Notch positions (ตำแหน่ง Notch ของแต่ละโรเตอร์) ──────────────────────
    //    เมื่อโรเตอร์อยู่ที่ Notch แล้วกดปุ่ม → โรเตอร์ถัดไปจะถูกดัน
    localparam [4:0] NOTCH_I   = 5'd24;   // Rotor I   notch = Y
    localparam [4:0] NOTCH_II  = 5'd12;   // Rotor II  notch = M
    localparam [4:0] NOTCH_III = 5'd21;   // Rotor III notch = V

    // ── Double-stepping detection (ก่อน advance) ─────────────────────────────
    //    กลไก Odometer ของ Enigma มีปรากฏการณ์ Double-Stepping:
    //    Middle rotor หมุนเมื่อ (1) Right ถึง notch  OR  (2) Middle อยู่ที่ notch ด้วย
    wire mid_at_notch = (pos2 == NOTCH_II);    // Middle ที่ notch ก่อน advance
    wire rgt_at_notch = (pos1 == NOTCH_III);   // Right  ที่ notch ก่อน advance

    // ── Next-position wires (combinational) ──────────────────────────────────
    //    สำคัญ: ใช้ pos_next ใน cipher datapath เพราะโรเตอร์หมุนก่อนไฟวิ่ง
    wire [4:0] pos1_next = keypress
                           ? ((pos1 == 5'd25) ? 5'd0 : pos1 + 5'd1)
                           : pos1;
    wire [4:0] pos2_next = (keypress & (rgt_at_notch | mid_at_notch))
                           ? ((pos2 == 5'd25) ? 5'd0 : pos2 + 5'd1)
                           : pos2;
    wire [4:0] pos3_next = (keypress & mid_at_notch)
                           ? ((pos3 == 5'd25) ? 5'd0 : pos3 + 5'd1)
                           : pos3;

    // ── Sequential: register positions & latch output ───────────────────────
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            pos1      <= init_pos1;
            pos2      <= init_pos2;
            pos3      <= init_pos3;
            char_out  <= 5'd0;
            valid_out <= 1'b0;
        end else begin
            pos1 <= pos1_next;
            pos2 <= pos2_next;
            pos3 <= pos3_next;
            if (keypress) begin
                char_out  <= cipher_out;   // latch combinational result
                valid_out <= 1'b1;
            end else begin
                valid_out <= 1'b0;
                // char_out holds last value
            end
        end
    end

    // ── Combinational cipher datapath ────────────────────────────────────────
    //    ใช้ pos_next เพื่อให้โรเตอร์หมุนก่อนที่สัญญาณจะผ่าน
    wire [4:0] pb_fwd_out;     // Plugboard (forward)
    wire [4:0] r3_fwd_out;     // Rotor III forward
    wire [4:0] r2_fwd_out;     // Rotor II  forward
    wire [4:0] r1_fwd_out;     // Rotor I   forward
    wire [4:0] ref_out;        // Reflector B
    wire [4:0] r1_bwd_out;     // Rotor I   backward
    wire [4:0] r2_bwd_out;     // Rotor II  backward
    wire [4:0] r3_bwd_out;     // Rotor III backward
    wire [4:0] cipher_out;     // Final encrypted letter

    // ─ ขาไป (Forward path) ──────────────────────────────────────────────────
    plugboard   u_pb_fwd (.char_in(char_in),    .char_out(pb_fwd_out));
    rotor  #(3) u_r3_fwd (.char_in(pb_fwd_out), .pos(pos1_next), .dir(1'b0), .char_out(r3_fwd_out));
    rotor  #(2) u_r2_fwd (.char_in(r3_fwd_out), .pos(pos2_next), .dir(1'b0), .char_out(r2_fwd_out));
    rotor  #(1) u_r1_fwd (.char_in(r2_fwd_out), .pos(pos3_next), .dir(1'b0), .char_out(r1_fwd_out));

    // ─ สะท้อน (Reflector) ───────────────────────────────────────────────────
    reflector   u_ref    (.char_in(r1_fwd_out), .char_out(ref_out));

    // ─ ขากลับ (Backward path) ───────────────────────────────────────────────
    rotor  #(1) u_r1_bwd (.char_in(ref_out),    .pos(pos3_next), .dir(1'b1), .char_out(r1_bwd_out));
    rotor  #(2) u_r2_bwd (.char_in(r1_bwd_out), .pos(pos2_next), .dir(1'b1), .char_out(r2_bwd_out));
    rotor  #(3) u_r3_bwd (.char_in(r2_bwd_out), .pos(pos1_next), .dir(1'b1), .char_out(r3_bwd_out));
    plugboard   u_pb_bwd (.char_in(r3_bwd_out), .char_out(cipher_out));

endmodule
// =============================================================================
//  END OF FILE
// =============================================================================