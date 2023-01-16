# q4 - A fast register-based interpreter/VM

## Paradigm
q4 is a register-based system. For q4, this means:
- There is an "accumulator" register, called ACC.
- The ACC is somewhat similar to TOS in a stack-based system.
- Naming a first-class register or specifying a constant sets the ACC.
- Operations may use and/or set the ACC.
- Operations can be chained.

There is no compilation in q4. The source code is executed directly.

## Registers
There are two types of registers, first-class and second-class:
- The first-class registers are (A-Z).
- The second-class registers are (a-z).
- Naming a first-class register sets the ACC.
- Both types can be set and can come after an operator (eg - ":<x>" or "+<a>").

## Operations
An operation can take 0, 1 or 2 operands:
- If an operation takes no operands, it just does whatever it does (eg - '"' or ';').
- If an operation takes one operand, the ACC is the operand (eg - '.' or '[').
- If an operation takes two operands, the ACC and (expr) are they (eg - '*' or '=').
- If an operation results in a value, that value goes into the ACC.
- Operations can be chained (eg - M*X+B:Y).

## Expressions
An (expr) can be either a register (first or second class), or a constant.

## Some examples: 
```
0(this is a comment)
1234              0(sets ACC=1234)
:G                0(sets register G=ACC ... 1234)
C.                0(sets ACC=register C, prints ACC as a decimal)
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
******* REGISTER OPERATIONS *******
[A-Z]       Set ACC = register[X].
:[A-z]      Set register[X] = ACC.
i[A-z]      Increment register[X].
d[A-z]      Decrement register[X].


******* INPUT/OUTPUT *******
[0-9]*      Parse a decimal number into ACC.
'(c)        Set the ACC to the ASCII value of (c).
"string"    Output "string".
.           Output the ACC as a decimal number.
.h          Output the ACC as a hex number.
.b          Output a single space.
.n          Output a new-line (ASCII 10).
,           Output the ACC as an ASCII character.


******* FUNCTIONS *******
::<X>...;;  Define function <X>.
^<X>        Call function <X>.
;           Return from function.


******* STACK OPERATIONS *******
s[A-Z]      Push/save register[X] onto the stack.
s.          Push/save ACC onto the stack.
r[A-Z]      Pop/restore register[X] from the stack.
r.          Pop/restore ACC from the stack.
r@          Copy the top of the stack to the ACC.


******* MATH *******
+(expr)     ACC = ACC + (expr).
-(expr)     ACC = ACC - (expr).
*(expr)     ACC = ACC * (expr).
/(expr)     ACC = ACC / (expr).


******* COMPARISONS *******
<(expr)     ACC = (ACC < (expr)) ? -1 : 0.
=(expr)     ACC = (ACC = (expr)) ? -1 : 0.
>(expr)     ACC = (ACC > (expr)) ? -1 : 0.


******* IF/THEN *******
(<code>)    IF: If (ACC = 0), skip to next ')'.


******* MEMORY OPERATIONS *******
!b(expr)    Set BYTES[(expr)]=ACC. This is an 8-bit operation.
!c(expr)    Set CELL[(expr)]=ACC. This is a (16/32/64-bit) operation.
!m(expr)    Set ABSOLUTE[(expr)]=ACC. This is an 8-bit operation.
@b          Set ACC=BYTES[ACC].
@c          Set ACC=CELLS[ACC].
@m          Set ACC=ABSOLUTE[ACC].

******* LOOPS *******
[           FOR LOOP: Set (count) = ACC. Save register[I]. Set register[I] to 0.
I           NOTE: Register[I] is the iteration counter for the current.
]           Increment register[I]. If (register[I] < (count)), jump back to the beginning.
{           WHILE LOOP: Begin.
}           End WHILE loop. If (ACC != 0), jump back to the beginning.
xU          Unwind the loop stack (use "(\u;)" to exit early from a loop).

******* OTHER *******
xM          Start address of the CELLS/BYTES area.
xH          Offset of the last used byte of code in the BYTES area.
xT          ACC = current clock() value.
`cmd`       Call system("cmd") (PC only).
xQ          Exit q4.
```
