// Q4 - a virtual stack machine/CPU

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "Q4.h"

byte isBye = 0, isError = 0;
char buf[100];
sys_t *sys;

#define T        DSTK[DSP]
#define N        DSTK[DSP-1]
#define R        RSTK[RSP]
#define DROP1    pop()
#define BASE     REG[1]
#define HERE     REG[7]

void vmReset() {
    DSP = RSP = LSP = 0;
    sys->mem = (long*)sys->bmem;
    sys->user = &BMEM[NUM_REGS*4];
    for (ulong i = 0; i < SZ_MEM; i++) { BMEM[i] = 0; }
    USER[HERE++] = ';';
    REG[ 1] = 10;                        // B (BASE)
    REG[12] = SZ_MEM;                    // M
    REG[17] = NUM_REGS;                  // R
    REG[18] = (long)sys;                 // S
    REG[20] = SZ_USER;                   // U
}

void vmInit(sys_t *theSystem) {
    sys = theSystem;
    vmReset();
}

void push(long v) { if (DSP < SZ_STK) { DSTK[++DSP] = v; } }
long pop() { return (0 < DSP) ? DSTK[DSP--] : 0; }

void rpush(addr v) { if (RSP < SZ_STK) { RSTK[++RSP] = v; } }
addr rpop() { return (0 < RSP) ? RSTK[RSP--] : 0; }

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

int regDigit(char x) {
    return isBetweenI(x, 'a', 'z') ? x - 'a' : -1;
}

long getRegNum(int pc, long &prn) {
    int rd = regDigit(USER[pc]);
    if (rd < 0) { prn = -1;  return pc; }
    prn = rd;
    rd = regDigit(USER[++pc]);
    if (rd < 0) { return pc; }
    prn = (prn * 26) + rd;
    rd = regDigit(USER[++pc]);
    if (rd < 0) { return pc; }
    prn = (prn * 26) + rd;
    return pc+1;
}

addr doDefineQuote(addr pc) {
    int depth = 1;
    push(pc);
    while ((pc < SZ_USER) && USER[pc]) {
        char c = USER[pc++];
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
        while ((pc < SZ_USER) && (USER[pc] != ')')) { pc++; }
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
    printStringF("\r\nUSER: size: %d bytes, HERE=%d", SZ_USER, HERE);
    if (HERE == 0) { printString("\r\n(no code defined)"); return; }
    addr x = HERE;
    int ti = 0, npl = 20;
    char txt[32];
    for (long i = 0; i < HERE; i++) {
        if ((i % npl) == 0) {
            if (ti) { txt[ti] = 0;  printStringF(" ; %s", txt); ti = 0; }
            printStringF("\n\r%05d: ", i);
        }
        txt[ti++] = (USER[i] < 32) ? '.' : USER[i];
        printStringF(" %3d", USER[i]);
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
    buf[0] = 'a' + (regNum / (26*26));
    buf[1] = 'a' + (regNum / 26 % 26);
    buf[2] = 'a' + (regNum % 26);
    buf[3] = 0;
    if (buf[0] == 'a') { 
        buf[0] = ' '; 
        if (buf[1] == 'a') { buf[1] = ' '; }
    }
    return buf;
}

void dumpRegs() {
    printStringF("\r\nREGISTERS: %d available", NUM_REGS);
    int n = 0;
    char buf[8];
    for (int i = 0; i < NUM_REGS; i++) {
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
    int ir = USER[pc++];
    switch (ir) {
#ifdef __PC__
    case 'C': if (T) { fclose((FILE*)T); }
        DROP1;
        break;
    case 'O': {
            byte *md = USER + pop();
            byte *fn = USER + T;
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
    int ir = USER[pc++];
    long pin = pop(), val = 0;
    switch (ir) {
    case 'I': pinMode(pin, INPUT); break;
    case 'U': pinMode(pin, INPUT_PULLUP); break;
    case 'O': pinMode(pin, OUTPUT); break;
    case 'R': ir = USER[pc++];
        if (ir == 'D') { push(digitalRead(pin)); }
        if (ir == 'A') { push(analogRead(pin)); }
        break;
    case 'W': ir = USER[pc++]; val = pop();
        if (ir == 'D') { digitalWrite(pin, val); }
        if (ir == 'A') { analogWrite(pin, val); }
        break;
    }
    return pc;
}

addr doExt(addr pc) {
    byte ir = USER[pc++];
    switch (ir) {
    case 'F': pc = doFile(pc);          break;
    case 'I': ir = USER[pc++];
        if (ir == 'A') { dumpAll(); }
        if (ir == 'C') { dumpCode(); }
        if (ir == 'R') { dumpRegs(); }
        if (ir == 'S') { dumpStack(0); }
        break;
    case 'K': ir = USER[pc++];
        if (ir == 'Y') { push(getChar()); }
        if (ir == '?') { push(charAvailable()); }
        break;
    case 'O': ir = USER[pc++];
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
    long t1, t2;
    isError = 0;
    while (!isError && (0 < pc)) {
        byte ir = USER[pc++];
        switch (ir) {
        case 0: RSP = 0; return -1;
        case ' ': while (USER[pc] == ' ') { pc++; }         // 32
                break;
        case '!': doStore(0, USER);                 break;  // 33
        case '"': buf[1] = 0;                               // 34
            while ((pc < SZ_USER) && (USER[pc] != '"')) {
                buf[0] = USER[pc++];
                printString(buf);
            }
            ++pc;                                   break;
        case '#': push(T);                          break;  // 35 (DUP)
        case '$': t1 = N; N = T; T = t1;            break;  // 36 (SWAP)
        case '%': push(N);                          break;  // 37 (OVER)
        case '&': t1 = pop(); T &= t1;              break;  // 38
        case '\'': push(USER[pc++]);                break;  // 39
        case '(': pc = doBegin(pc);                 break;  // 40
        case ')': pc = doWhile(pc);                 break;  // 41
        case '*': t1 = pop(); T *= t1;              break;  // 42
        case '+': t1 = pop(); T += t1;              break;  // 43
        case ',': buf[0] = (byte)pop(); buf[1] = 0;
            printString(buf);                       break;  // 44 (EMIT)
        case '-': t1 = pop(); T -= t1;              break;  // 45
        case '.': printStringF("%ld", pop());       break;  // 46
        case '/': t1 = pop();                               // 47
            if (t1) { T /= t1; }
            else { isError = 1; }
            break;  // 47
        case '0': case '1': case '2': case '3': case '4':   // 48-57
        case '5': case '6': case '7': case '8': case '9':
            push(ir - '0');
            t1 = USER[pc] - '0';
            while ((0 <= t1) && (t1 <= 9)) {
                T = (T * 10) + t1;
                t1 = USER[++pc] - '0';
            }
            break;
        case ':': /* FREE */                         break;  // 58
        case ';': if (USER[pc] == ';') { rpop(); }           // 59
                pc = rpop();
                if (pc == 0) { RSP = 0; return pc; }
                break;
        case '<': t1 = pop(); T = (T <  t1) ? 1 : 0; break;  // 60
        case '=': t1 = pop(); T = (T == t1) ? 1 : 0; break;  // 61
        case '>': t1 = pop(); T = (T >  t1) ? 1 : 0; break;  // 62
        case '?': /* FREE */                         break;  // 63
        case '@': doFetch(0, USER);                  break;
        case 'A': ir = USER[pc++]; t2 = 0;
            if (ir == 'C') { t2 = 1; ir = USER[pc++]; }
            if (ir == '@') { doFetch((byte)t2, 0); }
            if (ir == '!') { doStore((byte)t2, 0); }
            break;
        case 'B': printString(" ");                  break;
        case 'C': ir = USER[pc++];
            if (ir == '@') { doFetch(1, USER); }
            if (ir == '!') { doStore(1, USER); }
            break;
        case 'D': /* FREE */                         break;
        case 'E': /* FREE */                         break;
        case 'F': T = ~T;                            break;
        case 'G': pc = (addr)pop();                  break;
        case 'H': push(0);
            t1 = digitToNum(USER[pc], 0x10, 2);
            while (0 <= t1) {
                T = (T * 0x10) + t1;
                t1 = digitToNum(USER[++pc], 0x10, 2);
            } break;
        case 'I': doIJK(pc, 1);                      break;
        case 'J': doIJK(pc, 2);                      break;
        case 'K': T *= 1000;                         break;
        case 'L': N = N << T; DROP1;                 break;
        case 'M': T--;                               break; // (Minus 1)
        case 'N': printString("\r\n");               break;
        case 'O': T = -T;                            break; // (NEGATE)
        case 'P': T++;                               break; // (Plus 1)
        case 'Q': /* FREE */                         break;
        case 'R': N = N >> T; DROP1;                 break; // (RIGHT-SHIFT)
        case 'S': t2 = N; t1 = T;                           // (SLASHMOD)
            if (t1 == 0) { isError = 1; printString("-divByZero-"); }
            else { N = (t2 / t1); T = (t2 % t1); }
            break;
        case 'T': push(millis());                    break;
        case 'U': if (T < 0) { T = -T; }             break;
        case 'V': /* FREE */                         break;
        case 'W': delay(pop());                      break;
        case 'X': pc = doExt(pc);                    break;
        case 'Y': t1 = pop();                               // LOAD
            if (__PC__) {
                if (input_fp) { fclose(input_fp); }
                sprintf_s(buf, sizeof(buf), "block.%03ld", t1);
                fopen_s(&input_fp, buf, "rt");
            }
            else { printString("-l:pc only-"); }
            break;
        case 'Z': printString((char*)&USER[pop()]);  break;
        case '[': pc = doFor(pc);                    break;    // 91
        case '\\': DROP1;                            break;    // 92
        case ']': pc = doNext(pc);                   break;    // 93
        case '^': rpush(pc); pc = (addr)pop();       break;    // 94
        case '_': /* FALL THROUGH */                           // 95
        case '`': while (USER[pc] && (USER[pc] != ir))         // 96
            { USER[T++] = USER[pc++]; }
            ++pc; break;
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
        case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
        case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
        case 's': case 't': case 'u': case 'v': case 'w': case 'x':
        case 'y': case 'z': ir -= 'a';
            pc = getRegNum(pc-1, t1);
            if (isBetweenI(t1, 0, NUM_REGS-1)) { 
                push(REG[t1]);
                ir = USER[pc];
                if (ir == '+') { ++pc; ++REG[t1]; }
                if (ir == '-') { ++pc; --REG[t1]; }
                if (ir == ':') { DROP1; ++pc; REG[t1] = pop(); }
            } else {
                isError = 1;
                printString("-regOOB-");
            } break;
        case '{': if (pop()) { break; }                        // 123
            while (USER[pc++]) { 
                if (USER[pc - 1] == '}') break; 
            } break;
        case '|': t1 = pop(); T |= t1;               break;    // 124
        case '}': /* FREE */                         break;    // 125
        case '~': T = (T) ? 0 : 1;                   break;    // 126 (Logical NOT)
        }
    }
    return 0;
}

void setCodeByte(addr loc, char ch) {
    if ((0 <= loc) && (loc < SZ_USER)) { USER[loc] = ch; }
}

long registerVal(char *reg) {
    USER[HERE + 0] = reg[0];
    USER[HERE + 1] = reg[1];
    USER[HERE + 2] = reg[2];
    long val = 0;
    getRegNum(HERE, val);
    return val;
}
