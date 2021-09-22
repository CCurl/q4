// Q4 - a stack VM, inspired by Sandor Schneider's STABLE - https://w3group.de/stable.html

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "Q4.h"

#define RN_SZ    2

byte isBye = 0, isError = 0;
char buf[100];
FILE* input_fp = NULL;
sys_t *sys;

#define T        DSTK[DSP]
#define N        DSTK[DSP-1]
#define R        RSTK[RSP]
#define DROP1    pop()
#define DROP2    pop(); pop()
#define BASE     MEM[1]
#define HERE     MEM[7]

void vmReset() {
    DSP = RSP = LSP = 0;
    sys->mem = (long*)sys->bmem;
    sys->code = &BMEM[SZ_REG*4];
    for (ulong i = 0; i < SZ_MEM; i++) { BMEM[i] = 0; }
    CODE[HERE++] = ';';
    BASE = 10;
    REG[2] = SZ_CODE;             // C
    REG[12] = SZ_MEM;             // M
    REG[17] = SZ_REG;             // R
    REG[21] = SZ_CODE + SZ_REG;   // V
}

void vmInit(sys_t *theSystem) {
    sys = theSystem;
    vmReset();
}

void push(long v) { if (DSP < SZ_STK) { DSTK[++DSP] = v; } }
long pop() { return (DSP > 0) ? DSTK[DSP--] : 0; }

void rpush(addr v) { if (RSP < SZ_STK) { RSTK[++RSP] = v; } }
addr rpop() { return (RSP > 0) ? RSTK[RSP--] : 0; }

void doStore(byte isByte, byte *base) {
    long t = pop(), n = pop();
    if (isByte) { base[t] = (n & 0xff); }
    else {
        if (__PC__ || ((t%4) == 0)) { *(long *)&base[t] = n; }
        else {
            base[t++] = ((n) & 0xff);
            base[t++] = ((n >> 8) & 0xff);
            base[t++] = ((n >> 16) & 0xff);
            base[t++] = ((n >> 24) & 0xff);
        }
    }
}

void doFetch(byte isByte, byte * base) {
    if (isByte) { T = base[T]; }
    else {
        long t = T;
        T  = (base[t++]);
        T |= (base[t++] <<  8);
        T |= (base[t++] << 16);
        T |= (base[t++] << 24);
    }
}

void printStringF(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    printString(buf);
}

#define isBetweenX(n, lo, hi) ((lo <  n) && (n <  hi))
#define isBetweenI(n, lo, hi) ((lo <= n) && (n <= hi))

int digitToNum(char x, byte base, byte mode) {
    // mode: 1 => upper, 2 => either, 3 => lower
    int n = -1;
    if (isBetweenI(x, '0', '9')) { n = x - '0'; }
    if (isBetweenI(x, 'A', 'Z') && (mode <= 2)) { n = x - 'A' + 10; }
    if (isBetweenI(x, 'a', 'z') && (2 <= mode)) { n = x - 'a' + 10; }
    if (base <= n) { n = -1; }
    return n;
}

char numToDigit(short num, byte base, byte toUpper) {
    char c = num % base + '0';
    if ('9' < c) { c += 7 + (toUpper ? 0 : 32); }
    return c;
}

int regDigit(char x, byte isFirst) {
    int n = -1;
    if ( isFirst && isBetweenI(x, 'a', 'z')) { n = x - 'a'; }
    if (!isFirst) { n = digitToNum(x, 36, 3); }
    if (n < 0) { isError = 1; }
    return n;
}

short getRegNum(int pc, byte msg) {
    int c1 = regDigit(CODE[pc], 1);
    int c2 = regDigit(CODE[pc+1], 1);
    if (isError) {
        if (msg) { printStringF("-%c%c:BadReg-", CODE[pc], CODE[pc+1]); }
        return -1;
    }
    short n = (c2*26) + c1;
    if (SZ_REG <= n) {
        if (msg) { printStringF("-%d:RN_OOB-", n); }
        isError = 1;
        return -1;
    }
    return n;
}

addr doDefineQuote(addr pc) {
    int depth = 1;
    push(pc);
    while ((pc < SZ_CODE) && CODE[pc]) {
        char c = CODE[pc++];
        if (c == '{') { ++depth; }
        if (c == '}') {
            --depth;
            if (depth == 0) { return pc; }
        }
    }
    isError = 1;
    printString("-noQtEnd-");
    return pc;
}

addr doBegin(addr pc) {
    rpush(pc);
    if (T == 0) {
        while ((pc < SZ_CODE) && (CODE[pc] != ')')) { pc++; }
    }
    return pc;
}

addr doWhile(addr pc) {
    if (T) { pc = R; }
    else { DROP1;  rpop(); }
    return pc;
}

addr doFor(addr pc) {
    if (LSP < 4) {
        LOOP_ENTRY_T* x = &LSTK[LSP];
        LSP++;
        x->start = pc;
        x->to = pop();
        x->from = pop();
        x->end = 0;
        if (x->to < x->from) {
            push(x->to);
            x->to = x->from;
            x->from = pop();
        }
    }
    return pc;
}

addr doNext(addr pc) {
    if (LSP < 1) { LSP = 0; }
    else {
        LOOP_ENTRY_T* x = &LSTK[LSP - 1];
        ++x->from;
        x->end = pc;
        if (x->from <= x->to) { pc = x->start; }
        else { LSP--; }
    }
    return pc;
}

addr doIJK(addr pc, int mode) {
    push(0);
    if ((mode == 1) && (0 < LSP)) { T = LSTK[LSP-1].from; }
    if ((mode == 2) && (0 < LSP)) { T = LSTK[LSP-2].from; }
    if ((mode == 3) && (0 < LSP)) { T = LSTK[LSP-3].from; }
    return pc;
}

void dumpCode() {
    printStringF("\r\nCODE: size: %d bytes, HERE=%d", SZ_CODE, HERE);
    if (HERE == 0) { printString("\r\n(no code defined)"); return; }
    addr x = HERE;
    int ti = 0, npl = 20;
    char txt[32];
    for (long i = 0; i < HERE; i++) {
        if ((i % npl) == 0) {
            if (ti) { txt[ti] = 0;  printStringF(" ; %s", txt); ti = 0; }
            printStringF("\n\r%05d: ", i);
        }
        txt[ti++] = (CODE[i] < 32) ? '.' : CODE[i];
        printStringF(" %3d", CODE[i]);
    }
    while (x % npl) {
        printString("    ");
        x++;
    }
    if (ti) { txt[ti] = 0;  printStringF(" ; %s", txt); }
}

void dumpStack(int hdr) {
    if (hdr) { printStringF("\r\nSTACK: size: %d ", SZ_STK); }
    printString("(");
    for (int i = 1; i <= DSP; i++) { printStringF("%s%ld", (i > 1 ? " " : ""), DSTK[i]); }
    printString(")");
}

char *getRegName(short regNum, char *buf) {
    int slash = regNum / 26, mod = regNum % 26;
    buf[0] = 'a' + mod;
    buf[1] = 'a' + slash;
    buf[2] = 0;
    return buf;
}

void dumpRegs() {
    printStringF("\r\nREGISTERS: %d available", SZ_REG);
    int n = 0;
    char buf[3];
    for (int i = 0; i < SZ_REG; i++) {
        if (REG[i] == 0) { continue; }
        if (((n++) % 5) == 0) { printString("\r\n"); }
        printStringF("%s: %-12ld  ", getRegName(i, buf), REG[i]);
    }
}

void dumpAll() {
    dumpStack(1);   printString("\r\n");
    dumpRegs();     printString("\r\n");
    dumpCode();     printString("\r\n");
}

addr doFile(addr pc) {
    int ir = CODE[pc++];
    switch (ir) {
#ifdef __PC__
    case 'C':
        if (T) { fclose((FILE*)T); }
        DROP1;
        break;
    case 'O': {
        byte* md = BMEM + pop();
        byte* fn = BMEM + T;
        T = 0;
        fopen_s((FILE**)&T, (char *)fn, (char *)md);
    }
            break;
    case 'R': if (T) {
        long n = fread_s(buf, 2, 1, 1, (FILE*)T);
        T = ((n) ? buf[0] : 0);
        push(n);
    }
            break;
    case 'W': if (T) {
        FILE* fh = (FILE*)pop();
        buf[1] = 0;
        buf[0] = (byte)pop();
        fwrite(buf, 1, 1, fh);
    }
            break;
#endif
    }
    return pc;
}

addr doPin(addr pc) {
    int ir = CODE[pc++];
    long pin = pop(), val = 0;
    switch (ir) {
    case 'I': pinMode(pin, INPUT); break;
    case 'U': pinMode(pin, INPUT_PULLUP); break;
    case 'O': pinMode(pin, OUTPUT); break;
    case 'R': ir = CODE[pc++];
        if (ir == 'D') { push(digitalRead(pin)); }
        if (ir == 'A') { push(analogRead(pin)); }
        break;
    case 'W': ir = CODE[pc++]; val = pop();
        if (ir == 'D') { digitalWrite(pin, val); }
        if (ir == 'A') { analogWrite(pin, val); }
        break;
    }
    return pc;
}

addr doExt(addr pc) {
    byte ir = CODE[pc++];
    switch (ir) {
    case 'F': pc = doFile(pc);          break;
    case 'I': ir = CODE[pc++];
        if (ir == 'A') { dumpAll(); }
        if (ir == 'C') { dumpCode(); }
        if (ir == 'R') { dumpRegs(); }
        if (ir == 'S') { dumpStack(0); }
        break;
    case 'O': ir = CODE[pc++];
        if (ir == 'R') { N ^= T; DROP1; }
        break;
    case 'P': pc = doPin(pc);           break;
    case 'R': vmReset();                break;
    case 'S': DSP = 0;                  break;
    case 'T': isBye = 1;                break;
    }
    return pc;
}

addr run(addr pc) {
    long t1, t2, t3;
    byte* bp;
    isError = 0;
    while (!isError && (0 < pc)) {
        byte ir = CODE[pc++];
        //printf("\n-pc:%3ld,ir:%3d(%c),DSP:%d,RSP:%d,T:%ld-", pc-1,ir,ir?ir:'.',DSP,RSP,T);
        switch (ir) {
        case 0: RSP = 0; return -1;
        case ' ': while (CODE[pc] == ' ') { pc++; }         // 32
                break;
        case '!': doStore(0, BMEM);                 break;  // 33
        case '"': buf[1] = 0;                               // 34
            while ((pc < SZ_CODE) && (CODE[pc] != '"')) {
                buf[0] = CODE[pc++];
                printString(buf);
            }
            ++pc;                                   break;
        case '#': push(T);                          break;  // 35 (DUP)
        case '$': t1 = N; N = T; T = t1;            break;  // 36 (SWAP)
        case '%': push(N);                          break;  // 37 (OVER)
        case '&': t1 = pop(); T &= t1;              break;  // 38
        case '\'': push(CODE[pc++]);                break;  // 39
        case '(': pc = doBegin(pc);                 break;  // 40
        case ')': pc = doWhile(pc);                 break;  // 41
        case '*': t1 = pop(); T *= t1;              break;  // 42
        case '+': t1 = pop(); T += t1;              break;  // 43
        case ',': printStringF("%c", (char)pop());  break;  // 44 (EMIT)
        case '-': t1 = pop(); T -= t1;              break;  // 45
        case '.': printStringF("%ld", pop());       break;  // 46
        case '/': t1 = pop();                               // 47
            if (t1) { T /= t1; }
            else { isError = 1; }
            break;  // 47
        case '0': case '1': case '2': case '3': case '4':   // 48-57
        case '5': case '6': case '7': case '8': case '9':
            push(ir - '0');
            t1 = CODE[pc] - '0';
            while ((0 <= t1) && (t1 <= 9)) {
                T = (T * 10) + t1;
                t1 = CODE[++pc] - '0';
            }
            break;
        case ':': /* FREE */                         break;  // 58
        case ';': if (RSP < 2) { RSP = 0; return pc; }       // 59
            rpop();  pc = rpop();
            break;
        case '<': t1 = pop(); T = T < t1  ? 1 : 0;   break;  // 60
        case '=': t1 = pop(); T = T == t1 ? 1 : 0;   break;  // 61
        case '>': t1 = pop(); T = T > t1  ? 1 : 0;   break;  // 62
        case '?': t2 = pop(); t1 = pop(); t3 = pop();        // 63
            if ( t3 && t1) { rpush(pc); pc = (addr)t1; } // TRUE case
            if (!t3 && t2) { rpush(pc); pc = (addr)t2; } // FALSE case
            break;
        case '@': doFetch(0, BMEM);             break;
        case 'A': ir = CODE[pc++];
            if (ir == '@') { doFetch(1, 0); }
            if (ir == '!') { doStore(1, 0); }
            break;
        case 'B': printString(" ");             break;
        case 'C': ir = CODE[pc++];
            if (ir == '@') { doFetch(1, BMEM); }
            if (ir == '!') { doStore(1, BMEM); }
            break;
        case 'D': ir = CODE[pc++];
            if (ir == '@') { doFetch(1, CODE); }
            if (ir == '!') { doStore(1, CODE); }
            break;
        case 'E': /* FREE */                    break;
        case 'F': T = ~T;                       break;
        case 'G': pc = (addr)pop();             break;
        case 'H': push(0);
            t1 = digitToNum(CODE[pc], 0x10, 2);
            while (0 <= t1) {
                T = (T * 0x10) + t1;
                t1 = digitToNum(CODE[++pc], 0x10, 2);
            } break;
        case 'I': doIJK(pc, 1);                 break;
        case 'J': doIJK(pc, 2);                 break;
        case 'K': T *= 1000;                    break;
        case 'L': N = N << T; DROP1;            break;
        case 'M': ir = CODE[pc++];
            if (ir == '@') { doFetch(1, 0); }
            if (ir == '!') { doStore(1, 0); }
            break;
        case 'N': printString("\r\n");          break;
        case 'O': T = -T;                       break; // (NEGATE)
        case 'P': T++;                          break; // (INCREMENT)
        case 'Q': T--;                          break; // (DECREMENT)
        case 'R': N = N >> T; DROP1;            break; // (RIGHT-SHIFT)
        case 'S': t2 = N; t1 = T;                      // (SLASHMOD)
            if (t1 == 0) { isError = 1; }
            else { N = (t2 / t1); T = (t2 % t1); }
            break;
        case 'T': push(millis());               break;
        case 'U': if (T < 0) { T = -T; }        break; // (ABS)
        case 'V': /* FREE */                    break;
        case 'W': delay(pop());                 break;
        case 'X': pc = doExt(pc);               break;
        case 'Y': t1 = pop();                          // LOAD
            if (__PC__) {
                if (input_fp) { fclose(input_fp); }
                sprintf_s(buf, sizeof(buf), "block.%03ld", t1);
                fopen_s(&input_fp, buf, "rt");
            } else { printString("-l:pc only-"); }
            break;
        case 'Z':  if ((0 <= T) && ((ulong)T < SZ_MEM)) { 
            bp = &BMEM[pop()];
            printString((char*)bp); }
            break;
        case '[': pc = doFor(pc);               break;       // 91
        case '\\': DROP1;                       break;       // 92
        case ']': pc = doNext(pc);              break;       // 93
        case '^': rpush(pc); pc = (addr)pop();  break;       // 94
        case '_': push(T);                                   // 95 (S" variant)
            while (CODE[pc] && (CODE[pc] != '_')) { MEM[T++] = CODE[pc++]; }
            ++pc; MEM[T++] = 0;
            break;
        case '`':                                            // 96 (String C,)
            while (CODE[pc] && (CODE[pc] != '`')) { CODE[HERE++] = CODE[pc++]; }
            ++pc; break;
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
        case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
        case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
        case 's': case 't': case 'u': case 'v': case 'w': case 'x':
        case 'y': case 'z': ir -= 'a';
            t1 = getRegNum(pc-1, 1);
            pc++;
            if (!isError) {
                push(MEM[t1]);
                ir = CODE[pc];
                //printf("-reg:%c%c:%ld-", CODE[pc-2], CODE[pc-1], T);
                if (ir == '+') { ++pc; ++MEM[t1]; }
                if (ir == '-') { ++pc; --MEM[t1]; }
                if (ir == ':') { DROP1; ++pc; MEM[t1] = pop(); }
            } break;
        case '{': pc = doDefineQuote(pc);    break;    // 123
        case '|': t1 = pop(); T |= t1;       break;    // 124
        case '}': if (0 < RSP) { pc = rpop(); }        // 125
                else { RSP = 0; return pc; }
            break;
        case '~': T = (T) ? 0 : 1;           break;    // 126 (Logical NOT)
        }
    }
    return 0;
}

void setCodeByte(addr loc, char ch) {
    if ((0 <= loc) && (loc < SZ_CODE)) { CODE[loc] = ch; }
}

long registerVal(int reg) {
    if ((0 <= 'A') && (reg <= 'Z')) { return MEM[reg - 'A']; }
    if ((0 <= 'a') && (reg <= 'z')) { return MEM[reg - 'a']; }
    return 0;
}

addr functionAddress(const char *fn) {
    CODE[HERE+0] = fn[0];
    CODE[HERE+1] = fn[1];
    CODE[HERE+2] = fn[2];
    return getRegNum(HERE, 0);
}