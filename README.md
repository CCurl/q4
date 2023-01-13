# A register-based interpreter

There are 26 registers, [A-Z].
- Register 'A' is the "accumulator" (ACC).
- The ACC is used to store intermediate results.
- An <expr> can be either a reference to a register or a constant.

## Some examples: 
```
- 0(this is a comment)
- 1234 .A           0(prints "1234")
- 'Y,               0(prints "Y")
- M*X+B:Y           0(sets Y=M*X+B)
- .Y                0(prints register Y)
- %H"Hello World"$  0(define function H)
- #H                0(call H)
- \t:S #H \t-S .A   0(prints the elapsed time of function H)
```

## Reference
```
"string"    Print "string".
.<expr>     Print the value of <expr> as a decimal number.
,<expr>     Print the value of <expr> as an ASCII character.

%<A-Z>...$  Define function <X>.
#<A-Z>      Call function <X>.
;           Return from function.

<0-9>*      Parse a decimal number into ACC.
<A-Z>       Copy the value of register <X> into ACC.
:<A-Z>      Copy the value of ACC into register <X>.

+<expr>     ACC = ACC + <expr>.
-<expr>     ACC = ACC - <expr>.
*<expr>     ACC = ACC * <expr>.
/<expr>     ACC = ACC / <expr>.

^<A-Z>      Increment register <X>.
_<A-Z>      Decrement register <X>.

<(expr)     ACC = (ACC < (expr)) ? -1 : 0.
=(expr)     ACC = (ACC = (expr)) ? -1 : 0.
>(expr)     ACC = (ACC > (expr)) ? -1 : 0.

(<code>)    IF: If (ACC = 0), skip to next ')'.

!<expr>     Store ACC to address <expr>. (eg - "X !3456" stores X to [3456])
@<expr>     ACC = value at address <expr>. (eg = "@22" gets the value at [22])

[           FOR LOOP: Set <count> = ACC. Initialize \i to 0.
\i          Iteration counter of the current FOR loop (range: 0 to <expr>-1).
]           Increment \i. If (\i < <count>), jump back to the beginning.

{           WHILE LOOP: Begin.
}           End WHILE loop. If (ACC != 0), jump back to the beginning.

\t          ACC = current clock() value.
\u          Unwind the current FOR or WHILE loop.
\q          Exit Q4.
```
