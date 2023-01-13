# A register-based interpreter

There are 26 registers, [A-Z].
Register 'A' is the "accumulator" (ACC).
The ACC is used to store intermediate results.
An <expr> can be either a reference to a register or a constant.

## Some examples: 
```
- "1234 .A" prints "1234".
- "66," prints "B".
- "M*X+B .A" prints the result of (M*X)+B.
- "\t:S <work> \t-S .A" prints the elapsed time of <work>.
```

## Reference
```
.<expr>     Print the value of <expr> as a decimal number.
,<expr>     Print the value of <expr> as an ASCII character.

<0-9>*      Parse a decimal number into ACC.
<A-Z>       Copy the value of register <X> into ACC.
:<A-Z>      Copy the value of ACC into register <X>.

+<expr>     ACC = ACC + <expr>.
-<expr>     ACC = ACC - <expr>.
*<expr>     ACC = ACC * <expr>.
/<expr>     ACC = ACC / <expr>.

!<expr>     Store ACC to address <expr>. (eg - "22!3456" stores 22 to [3456])
@<expr>     Get value at address <expr> into ACC. (eg = "@3456" gets the value at [22])

[           Begin FOR loop. Execute the loop ACC times. Initialize \i to 0.
\i          Iteration counter of the current FOR loop (range: 0 to <expr>-1).
]           Increment \i. If less than <expr>, jump back to the beginning.

{           Begin WHILE loop.
}           End WHILE loop. If ACC != 0, jump back to the beginning.

\t          The current clock() value
\u          Unwind the current loop.
~           Exit
```
