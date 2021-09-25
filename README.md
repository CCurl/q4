# Q4 - A Virtual Forth CPU

Q4 is an implementation of a Virtual Forth CPU whose machine code is human-readable. As such, there is no need for a compiler or assembler.

Why Q4? There are multiple reasons:

1. I wanted a minimal program with a small footprint that could be easily configured and deployed to different development boards via the Arduino IDE.

2. I wanted a simple and interactive programming environment that could be easily understood and modified.

3. I wanted to avoid the need for using a multiple gigabyte tool chain and the edit/compile/deploy/run paradigm for developing programs.

4. I wanted short commands so there was not a lot of typing needed.

Q4 is the result of my work towards those goals.

- The entire system is implemented 4 files: Q4.h, Q4.cpp, pc-main.cpp, and Q4.ino.
- The same code runs on both a Windows PC or development board via the Arduino IDE. 

The reference for Q4 is here:   https://github.com/CCurl/Q4/blob/main/reference.txt

There are examples for Q4 here: https://github.com/CCurl/Q4/blob/main/examples.txt

# Building Q4

- On the PC, I use Microsoft's Visual Studio (Community edition). 
- For Development boards, I use the Arduino IDE. 
- I do not have an Apple or Linux system, so I have not tried to compile Q4 in those environments.
- However, being such a simple and minimaL C program, it should not be difficult to port Q4 to those environments.
