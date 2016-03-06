// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Fill.asm

// Runs an infinite loop that listens to the keyboard input. 
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel. When no key is pressed, the
// program clears the screen, i.e. writes "white" in every pixel.

// Put your code here.
    // screen_end is set to final screen memory location.
    @SCREEN
    D=A
    @8192
    D=D+A
    @screen_end
    M=D    
    (START)
    // screen_start is initialized to the first screen memory location.
    @SCREEN
    D=A
    @screen_current
    M=D
    // Read a key from the keyboard.
    @KBD
    D=M
    @KEY_PRESSED
    D;JNE
    // Keyboard reads 0, key not pressed, set color to clear.
    @color
    M=0
    @FILL
    0;JMP
    (KEY_PRESSED)
    // Keyboard reads non-0, key pressed, set color to black.
    @color
    M=-1
    (FILL)
    // Write color to the current screen location.
    @color
    D=M
    @screen_current
    A=M
    M=D
    // Update screen_current to the next memory location.
    @screen_current
    MD=M+1
    // If the final location is reached, listen for another keypress, else contine to
    // fill.
    @screen_end
    D=D-M
    @START
    D;JEQ
    @FILL
    0;JMP
    // Never reached.
    (END)
    @END
    0;JMP
