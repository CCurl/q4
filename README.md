# A register-based interpreter

There are 26 registers, [A-Z].
- Register 'A' is the "accumulator" (ACC).
- The ACC is used to store intermediate results.
- An <expr> can be either a reference to a register or a constant.

## Some examples: 
```
- 0(this is a comment)
- 1234.             0(prints "1234")
- 'Y,               0(prints "Y")
- M*X+B:Y           0(sets Y=M*X+B)
- Y.                0(prints register Y)
- %H"Hello";;       0(define function H)
- #H                0(call function H)
- xT:S #H xT-S.     0(prints the elapsed time of function H)
```

## Reference
```
"string"    Print "string".
.           Print the value of ACC as a decimal number.
,           Output ACC as an ASCII character.
'<char>     Set ACC to the ASCII value for <char>

::<X>...;;  Define function <X>.
_<X>        Call function <X>.
;           Return from function.

<0-9>*      Parse a decimal number into ACC.
<A-Z>       Copy the value of register <X> into ACC.
:<A-Z>      Copy the value of ACC into register <X>.

++<A-Z>     Increment register <X>.
--<A-Z>     Decrement register <X>.

+<expr>     ACC = ACC + <expr>.
-<expr>     ACC = ACC - <expr>.
*<expr>     ACC = ACC * <expr>.
/<expr>     ACC = ACC / <expr>.

<(expr)     ACC = (ACC < (expr)) ? -1 : 0.
=(expr)     ACC = (ACC = (expr)) ? -1 : 0.
>(expr)     ACC = (ACC > (expr)) ? -1 : 0.

(<code>)    IF: If (ACC = 0), skip to next ')'.

!<expr>     Store ACC to address <expr> (eg - "340:B X!B" stores X to [340]).
@           ACC = value at [ACC] (eg = "200@" gets the value at [200]).

[           FOR LOOP: Set <count> = ACC. Initialize i to 0.
i           Iteration counter of the current FOR loop (range: 0 to <expr>-1).
]           Increment i. If (i < <count>), jump back to the beginning.

{           WHILE LOOP: Begin.
}           End WHILE loop. If (ACC != 0), jump back to the beginning.

xB          Output a single space.
xB          Output a new-line (ASCII 10).
xT          ACC = current clock() value.
xU          Unwind the loop stack (use "(\u;)" to exit early from a loop).
xQ          Exit Q4.
```
