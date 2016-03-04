// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Mult.asm

// Multiplies R0 and R1 and stores the result in R2.
// (R0, R1, R2 refer to RAM[0], RAM[1], and RAM[2], respectively.)

// Put your code here.
    @R2
    M=0
    @R0
    D=M
    @R1
    D=D-M
    @R1_MIN
    D;JLT
    // R1 >= R0
    @R1
    D=M
    @max
    M=D
    @R0
    D=M
    @min
    M=D
    @MULTIPLY
    0;JMP
    (R1_MIN)
    // R1 < R0
    @R1
    D=M
    @min
    M=D
    @R0
    D=M
    @max
    M=D
    (MULTIPLY)
    // Add max value to itself min times.
    @min
    D=M
    @END
    D;JEQ
    @max
    D=M
    @R2
    M=D+M
    @min
    M=M-1
    @MULTIPLY
    0;JMP
    (END)
    @END
    0;JMP
