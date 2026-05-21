`timescale 1ns/1ps
module enigma_tb;

    reg        clk, rst_n, keypress;
    reg  [4:0] char_in;
    wire [4:0] char_out;
    wire       valid_out;
    wire [4:0] dbg_pos1, dbg_pos2, dbg_pos3;

    enigma_machine uut (
        .clk(clk), .rst_n(rst_n),
        .keypress(keypress),
        .char_in(char_in),
        .init_pos1(5'd0), .init_pos2(5'd0), .init_pos3(5'd0),
        .char_out(char_out), .valid_out(valid_out),
        .dbg_pos1(dbg_pos1), .dbg_pos2(dbg_pos2), .dbg_pos3(dbg_pos3)
    );

    always #5 clk = ~clk;

    // กด 1 ปุ่ม แล้วรอผลลัพธ์
    task press;
        input [4:0] ch;
        begin
            @(negedge clk);
            char_in  = ch;
            keypress = 1;
            @(posedge clk); #1;
            keypress = 0;
            @(posedge clk); #1;
            $display("  %s (%2d)  ->  %s (%2d)   [pos III=%0d II=%0d I=%0d]",
                65+ch, ch, 65+char_out, char_out,
                dbg_pos1, dbg_pos2, dbg_pos3);
        end
    endtask

    // ── ใส่คำที่ต้องการเข้ารหัสตรงนี้ ─────────────────────────────────────────
    // แต่ละตัวอักษร: A=0, B=1, ... Z=25  (ใช้ตัวพิมพ์ใหญ่เท่านั้น)
    parameter MSG_LEN = 5;
    reg [4:0] message [0:MSG_LEN-1];

    integer i;
    reg [4:0] encoded [0:MSG_LEN-1];

    initial begin
        // ── กำหนดข้อความที่นี่ ─────────────────────────────────────────────────
        message[0] = "H" - 65;   // H
        message[1] = "E" - 65;   // E
        message[2] = "L" - 65;   // L
        message[3] = "L" - 65;   // L
        message[4] = "O" - 65;   // O
        // ──────────────────────────────────────────────────────────────────────

        $dumpfile("enigma.vcd");
        $dumpvars(0, enigma_tb);

        clk = 0; rst_n = 0; keypress = 0; char_in = 0;
        #20; rst_n = 1; #10;

        // ── Encode ────────────────────────────────────────────────────────────
        $display("\n=== Encode ===");
        $display("  IN -> OUT  [Rotor positions]");
        for (i = 0; i < MSG_LEN; i = i+1) begin
            press(message[i]);
            encoded[i] = char_out;
        end

        // ── Decode (reset แล้วป้อน ciphertext กลับ) ──────────────────────────
        $display("\n=== Decode (should recover original) ===");
        rst_n = 0; #20; rst_n = 1; #10;
        for (i = 0; i < MSG_LEN; i = i+1)
            press(encoded[i]);

        $display("\n=== Done ===");
        $finish;
    end

endmodule
