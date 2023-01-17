# q4 - A fast register-based interpreter/VM

## Paradigm
q4 is a human-readable machine language for a virtual CPU.
- There is an "accumulator" register, called ACC.
- The ACC is somewhat similar to TOS in a stack-based system.
- There are 26 first-class registers, and 26 second-class registers.
- Naming a first-class register or specifying a constant sets the ACC.
- Operations may use and/or set the ACC.
- Operations can be chained.

There is no compilation in q4. The machine language is executed directly.

## Registers
There are two types of registers, first-class and second-class.
- The first-class registers are (A-Z).
- The second-class registers are (a-z).
- Naming a first-class register sets the ACC.
- Both types can be set and can come after an operator (eg - ":<x>" or "+<a>" or "mXY").

## Operations
An operation is just an opcode with some number of operands:
- There is NO space between an opcode and its operand(s).
- The list of opcodes is in the reference below.
- Operations can be chained (eg - M*X+B:Y).

## Expressions
An (expr) can be:
- A register (first or second class).
- A decimal constant.
- '.', which means the ACC (eg - "25*." sets ACC=25*25).

## Some examples: 
```
0(this is a comment)
1234              0(sets ACC=1234)
:G                0(sets register G=ACC)
..                0(prints ACC as a decimal)
.C                0(prints register C as a decimal)
34-12             0(sets ACC=34, sets ACC=ACC-12)
'Y,               0(sets ACC=89, prints the ACC ... "Y")
M*X+B:Y           0(sets Y=M*X+B)
"Hello"           0(prints "Hello")
::H1000*.[^A];;   0(define function H)
^H                0(call function H)
xT:S ^H xT-.S     0(prints the elapsed time of function H)
sG rC             0(Copies register G to C using the stack)
mXY               0(Moves/copies register X to Y, leaves ACC alone)
25*. ..           0(Sets ACC=15, sets ACC=ACC*ACC, prints ACC)
```

## Reference
```
******* REGISTER OPERATIONS *******
[A-Z]       Set ACC = register[X]. First-class registers only.
:[A-z]      Set register[X] = ACC.
i[A-z]      Increment register[X].
d[A-z]      Decrement register[X].
mAB         Move/copy register A to B. Does not affect the ACC.


******* INPUT/OUTPUT *******
[0-9]*      Parse a decimal number into ACC.
'(c)        Set the ACC to the ASCII value of (c).
"string"    Output "string".
.(expr)     Output (expr) as a decimal number.
.h(expr)    Output (expr) as a hex number.
.b          Output a single space.
.n          Output a new-line (ASCII 10).
,(expr)     Output (expr) as an ASCII character.


******* FUNCTIONS *******
::X...;;    Define function X.
^X          Call function X.
;           Return from function.


******* STACK OPERATIONS *******
s(expr)     Save (expr) onto the stack.
r[A-z]      Restore/pop TOS to register[X].
r.          Restore/pop TOS to the ACC.
r@          Copy the TOS to the ACC.


******* MATH *******
+(expr)     ACC = ACC + (expr).
-(expr)     ACC = ACC - (expr).
*(expr)     ACC = ACC * (expr).
/(expr)     ACC = ACC / (expr).
%(expr)     ACC = ACC % (expr).


******* COMPARISONS *******
<(expr)     ACC = (ACC < (expr)) ? -1 : 0.
=(expr)     ACC = (ACC = (expr)) ? -1 : 0.
>(expr)     ACC = (ACC > (expr)) ? -1 : 0.


******* IF/THEN *******
(<code>)    IF: If (ACC = 0), skip to next ')'.


******* MEMORY OPERATIONS *******
            NOTE: CELLS and BYTES share the same memory.
                - BYTE index B refers to the same location as CELL index B/(cell-size).
                - CELL index C refers to the same location as BYTE index C*(cell-size).
                - eg: for a (cell-size) of 4, CELLS[10] is BYTES[40-43].
!b(expr)    Set BYTES[(expr)]=ACC. This is an 8-bit operation.
!c(expr)    Set CELLS[(expr)]=ACC. This is a (cell-size) operation.
!m(expr)    Set ABSOLUTE[(expr)]=ACC. This is an 8-bit operation.
@b(expr)    Set ACC=BYTES[(expr)].
@c(expr)    Set ACC=CELLS[(expr)].
@m(expr)    Set ACC=ABSOLUTE[(expr)].

******* LOOPS *******
[           FOR LOOP: Set (count) = ACC. Save register[I]. Set register[I] to 0.
I           NOTE: Register[I] is the iteration counter for the current.
]           Increment register[I]. If (register[I] < (count)), jump back to the beginning.
{           WHILE LOOP: Begin.
}           End WHILE loop. If (ACC != 0), jump back to the beginning.
xU          Unwind the loop stack (use "(\u;)" to exit early from a loop).

******* OTHER *******
9/10/13/32  These ASCII chars are no-ops, and are optional.
xM          Start address of the CELLS/BYTES area.
xH          Index to the next free byte in the BYTES area.
xT          ACC = current clock() value.
`cmd`       Call system("cmd") (PC only).
xQ          Exit q4.
```
