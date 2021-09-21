# Q4 - A small and fast stack machine VM

Q4 is an simple, fast, and minimal interpreted environment where the source code IS the executable code. There is no compilation needed.

Why Q4? There are multiple reasons:

1. I wanted a program with a small footprint that could be easily configured and deployed to different development boards via the Arduino IDE.

2. Many programming environments use tokens and a large SWITCH statement in a loop to run the user program. In those systems, the tokens (the cases in the SWITCH) are often arbitrarily assigned and are not human-readable, so they have no meaning to the programmer when reading the machine code. Additionally there is a compiler, often something similar to Forth, to work in that environment. In these enviromnents, there is a steep learning curve. The programmer needs to learn the user environment and the hundreds or thousands of user functions in the libraries (or "words" in Forth). In Q4, there is only one thing to learn; THE SOURCE CODE IS THE MACHINE CODE and THE ASSEMBLER CODE, all in one. There is no compiler whatsoever, and there aren't thousands of functions/words to learn in order to use it.

3. I wanted a simple, minimal, and interactive programming environment that could be easily understood and modified.

4. I wanted to avoid the need for using a multiple gigabyte tool chain and the edit/compile/run paradigm for developing programs.

5. I wanted short commands so there was not a lot of typing needed.

Q4 is the result of my work towards those goals.

- The entire system is implemented 4 files: Q4.h, Q4.cpp, pc-main.cpp, and Q4.ino.
- The same code runs on both a Windows PC or development board via the Arduino IDE. 

The reference for Q4 is here:   https://github.com/CCurl/Q4/blob/main/reference.txt

There are examples for Q4 here: https://github.com/CCurl/Q4/blob/main/examples.txt

# Building Q4

- On the PC, I use Microsoft's Visual Studio (Community edition). 
- For Development boards, I use the Arduino IDE. 
- I do not have an Apple or Linux system, so I haven't tried to compile the program for those environments
- However, being such a simple C program, it should not be difficult to port Q4 to those environments.

Q4 was inspired by STABLE. See https://w3group.de/stable.html for details on STABLE.
A big thanks to Sandor Schneider for the inspiration for this project.
