# Q4 - A fast register-based interpreter

## Registers
- Register 'a' is the "accumulator" (ACC).
- There are two types of registers: first-class and second-class.
- There are 26 first-class registers, [A-Z].
- All other characters are "second-class" registers.
- For example, (e, x, $, %, and _) are all second-class registers.
- This is just a lucky side-effect of the way Q4 is implemented.
- Only first-class registers can come before an operator (start things off).
- Both can be set from the ACC and come after an operator.

## Expressions
- An <expr> can be either a register (first or second class), or a constant.
- For example: (A, x, _, and 1234) are all valid expressions.

## Operations
- A constant or a first-class register can be used to initialize the ACC.
- Simply specifying one sets the ACC (eg - X or 3345).
- Operations that take 1 parameter act on ACC.
- Operations that take 2 parameters use ACC on the left and <expr> on the right.
- For example: F-3 sets ACC to F, then subtracts 3 from ACC.
- Operations can be chained (eg - M*X+B).

## Some examples: 
```
0(this is a comment)
1234              0(sets ACC=1234)
:G                0(sets register G=ACC ... 1234)
G.                0(sets ACC=register G, prints ACC ... "1234")
34-12             0(sets ACC=34, sets ACC=ACC-12)
'Y,               0(sets ACC=89, prints the ACC ... "Y")
M*X+B:Y           0(sets Y=M*X+B)
"Hello"           0(prints "Hello")
::H"Hello";;      0(define function H)
^H                0(call function H)
xT:S ^H xT-S.     0(prints the elapsed time of function H)
```

## Reference
```
"string"    Print "string".
.           Print the value of ACC as a decimal number.
,           Output ACC as an ASCII character.
'<char>     Set ACC to the ASCII value for <char>

::<X>...;;  Define function <X>.
^<X>        Call function <X>.
;           Return from function.

<0-9>*      Parse a decimal number into ACC.
<A-Z>       Set ACC = register <X>.
:<A-z>      Set register <X> = ACC.

++<A-z>     Increment register <X>.
--<A-z>     Decrement register <X>.

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
