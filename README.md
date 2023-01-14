# Q4 - A fast register-based interpreter

## Paradigm
Q4 is a register-based system. For Q4, this means:
- There is an "accumulator" register (ACC).
- The ACC is similar to TOS in a stack-based system.
- A constant sets the ACC.
- Naming a first-class register sets the ACC.
- If an operation results in a value, that value goes into the ACC.

## Registers
There are two types of registers, first-class and second-class:
- The first-class registers are (A-Z).
- The second-class registers are (a-z).
- Naming a first-class register sets the ACC.
- Both types can be set and can come after an operator (":<x>" or "+<a>").

## Operations
An operation takes either 0, 1 or 2 operands:
- Operations that take no operands just do whatever they do (eg - '"' or ';').
- Operations that take one operand use the ACC for that operand (eg - '.' or ',').
- Operations that take two operands use the ACC and (expr) (eg - '*' or '=').
- If an operation results in a value, that value goes into the ACC.
- Operations can be chained (eg - M*X+B:Y).

## Expressions
An (expr) can be either a register (first or second class), or a constant.

## Some examples: 
```
0(this is a comment)
1234              0(sets ACC=1234)
:G                0(sets register G=ACC ... 1234)
X.                0(sets ACC=register X, prints ACC as a decimal)
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

[0-9]*      Parse a decimal number into ACC.
<A-Z>       Set ACC = register <X>.
:<A-z>      Set register <X> = ACC.

s+          Push ACC to the the stack.
s-          Pop ACC from the stack.
s@          Copy TOS to the ACC.

++<A-z>     Increment register <X>.
--<A-z>     Decrement register <X>.

+(expr)     ACC = ACC + (expr).
-(expr)     ACC = ACC - (expr).
*(expr)     ACC = ACC * (expr).
/(expr)     ACC = ACC / (expr).

<(expr)     ACC = (ACC < (expr)) ? -1 : 0.
=(expr)     ACC = (ACC = (expr)) ? -1 : 0.
>(expr)     ACC = (ACC > (expr)) ? -1 : 0.

(<code>)    IF: If (ACC = 0), skip to next ')'.

!(expr)     Store ACC to address (expr) (eg - "340:B X!B" stores X to [340]).
@           ACC = value at [ACC] (eg = "200@" gets the value at [200]).

[           FOR LOOP: Set <count> = ACC. Initialize i to 0.
i           Iteration counter of the current FOR loop (range: 0 to (expr)-1).
]           Increment i. If (i < <count>), jump back to the beginning.

{           WHILE LOOP: Begin.
}           End WHILE loop. If (ACC != 0), jump back to the beginning.

xB          Output a single space.
xB          Output a new-line (ASCII 10).
xT          ACC = current clock() value.
xU          Unwind the loop stack (use "(\u;)" to exit early from a loop).
xQ          Exit Q4.
```
