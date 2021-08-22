// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Fill.asm

// Runs an infinite loop that listens to the keyboard input.
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel;
// the screen should remain fully black as long as the key is pressed. 
// When no key is pressed, the program clears the screen, i.e. writes
// "white" in every pixel;
// the screen should remain fully clear as long as no key is pressed.

// Put your code here.
(INIT)
    @SCREEN
	D=A
	@4
	M=D
(LOOP)
	// Choose color
	@24576
	D=M
	@WHITE
	D;JEQ

(BLACK)
	@4
	D=M
	A=D
	M=-1

	@CONTINUE
	0;JMP

(WHITE)
	@4
	D=M
	A=D
	M=0

	@CONTINUE
	0;JMP

(CONTINUE)
	@4
	M=D+1

	@24576
	D=A-D

	@LOOP
	D;JGT

	@INIT
	D;JMP
   

