# Q4 - A register-based interpreter

What is Q4? Q4 is an implementation of a register-based VM.

Why Q4? I have multiple goals:

- A simple, minimal, and interactive programming environment that can be easily understood and modified.
- A program that can be used on both a Windows/Linux PCs.
- An environment that can be configured and deployed to different development boards.
- No need to use a multiple gigabyte tool chain.
- Break out of the edit/compile/deploy/run loop for developing programs.

Q4 is the result of my work towards those goals.

## Building Q4

- On Windows, I use Microsoft's Visual Studio (Community edition). 
- On Linux (Mint), I use gcc (or clang). 
- For Development boards, I use the Arduino IDE. 
- I do not have an Apple, so I have not tried to compile Q4 in that environment.
- However, being a simple and minimal C program, it should be easy to port Q4 to other environments.

## -  Q4 Reference
```
//                  Comment to the end of the line.
def <XXX>           Define address label <XXX>.
<XXX>               Call address <XXX>.
<XXX> goto          Goto address <XXX>.
ret                 Return from the current function.
                    Note: There can be many returns from one function.
                          Any locals declared will the discarded (see l+).
n if <x> then       If n != 0, perform <x>.
                    Note: 'else' is not supported.
                          <x> can be any number of statements.
+CAB                Set rC to (rA + rB).
-CAB                Set rC to (rA - rB).
*CAB                Set rC to (rA * rB).
/CAB                Set rC to (rA / rB).
mBA                 Set rB to rA (shorthand for rA sB).
[ <x> ] (t f--)     FOR: Perform <x> .
I         (--n)     The index of the current FOR loop.
begin <x> n while   Perform <x> while n!=0.
begin <x> n until   Perform <x> while n==0.
begin <x> repeat    Perform <x> forever.
'X      (--n)       Push the ascii value of X.
.       (n--)       Print TOS in decimal.
.R                  Print rR in decimal.
.RB                 Print rR in decimal, followed by a BLANK.
,       (c--)       Print c as a character.
,X                  Print rX as a character.
B                   Print a blank.
N                   Print a NewLine.
[0..9]*             Specify a value in decimal.
                    Note: If followed by sX, the value is placed into rX.
                          Else, the value is pushed onto the stack.
sX      (n--)       Set rX to the last number scanned.
rX      (--n)       Push rX onto the stack.
iX                  Increment rX.
dX                  Decrement rX.
=    (a b--f)       f: -1 if a==b, else 0.
<    (a b--f)       f: -1 if a<b, else 0.
>    (a b--f)       f: -1 if a>b, else 0.
l+      (n--)       Define n locals.
                    Note: The locals are automatically discarded when the function returns.
tX                  Set rX to the the current clock() value.
xQ                  Exit Q4.
```
## - Examples
```
(todo)
```