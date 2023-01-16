// q4.cpp - a fast register-based interpreter

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>


#define STK_SZ             32
#define LSTK_SZ            11
#define MEM_SZ          10000
#define CODE_SZ          1000
#define REGS_SZ     'z'-'A'+1
#define FUNCS_SZ    'Z'-'A'+1

#define BTW(a,b,c) ((b<=a) && (a<=c))

#define ACC          acc
#define RG(x)        regs[(x)-'A']

#define PC           *(pc)
#define IR           *(pc-1)
#define NR           *(pc++)

#define L0           lstk[lsp]
#define L1           lstk[lsp-1]
#define L2           lstk[lsp-2]
#define LU           lsp = (lsp<3) ? 0 : (lsp-3)

#define BYTES(x)      mem.b[x]
#define CELLS(x)      mem.c[x]

typedef long cell_t;
union { cell_t c[MEM_SZ/sizeof(cell_t)]; char b[MEM_SZ]; } mem;

cell_t regs[REGS_SZ], lstk[LSTK_SZ+1];
char *funcs[FUNCS_SZ], *stk[STK_SZ], *pc, *here, isBye;
cell_t acc, sp, lsp, t1;
FILE *input_fp;

void init() {
    here = &BYTES(0);
    *(here++) = ';';
}

// inline float toFlt(int x) { return *(float*)&x; }
// inline int toInt(float x) { return *(int*)&x; }

long expr() {
    if (BTW(PC, '0', '9')) {
        t1 = NR - '0';
        while (BTW(PC, '0', '9')) { t1 = (t1 * 10) + NR - '0'; }
        return t1;
    }
    // if (PC=='\'') { ++pc; return NR; } -- TOO SLOW!
    return RG(NR);
}

#define NEXT goto next
void Run2(const char *x) {
    sp = lsp = 0;
    pc = (char *)x;
next:
    switch (NR) {
    case 0: return;
    case 9: case 10: case13: case ' ': NEXT;
    case '!': t1=NR;
            if (t1=='c') { CELLS(expr()) = ACC; }
            else if (t1=='b') { BYTES(expr()) = (char)ACC; }
        NEXT;
    case '"': while (PC!='"') { putchar(NR); } ++pc; NEXT;
    // '#' .. '&': free;
    case '\'': ACC = NR; NEXT;
    case '(': if (!ACC) { while (NR != ')') { ; } } NEXT;
    case ')': NEXT;
    case '*': ACC *= expr(); NEXT;
    case '+': if (PC == '+') { ++pc; ++RG(NR); } else { ACC += expr(); } NEXT;
    case ',': putchar((int)ACC); NEXT;
    case '-': if (PC == '-') { ++pc; ++RG(NR); } else { ACC -= expr(); } NEXT;
    case '.': printf("%ld", (long)ACC); NEXT;
    case '/': ACC /= expr(); NEXT;
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
        --pc; ACC = expr(); NEXT;
    case ':': if (PC != ':') { RG(NR) = ACC; NEXT; }
        ++pc; funcs[PC-'A'] = pc+1;
        while (PC) {
            if ((PC==';') && (IR==';')) { break; }
            else { ++pc; }
        } ++pc; here=pc; NEXT;
    case ';': if (0 < sp) { pc = stk[sp--]; } else { sp = 0; pc = 0; } NEXT;
    case '<': ACC = (ACC <  expr()) ? -1 : 0; NEXT;
    case '=': ACC = (ACC == expr()) ? -1 : 0; NEXT;
    case '>': ACC = (ACC >  expr()) ? -1 : 0; NEXT;
    case '?': NEXT;
    case '@': t1=NR;
            if (t1=='c') { ACC = CELLS(ACC); }
            else if (t1=='b') { ACC = BYTES(ACC); }
            NEXT;
    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
    case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
    case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
    case 'V': case 'W': case 'X': case 'Y': case 'Z': ACC = RG(IR);  NEXT;
    case '[': lsp+=3; L0=0; L1=ACC; L2=(cell_t)pc; NEXT;
    case ']': if (++L0<L1) { pc=(char *)L2; } else { LU; } NEXT;
    case '^': stk[++sp]=pc+1; pc=funcs[PC-'A']; NEXT;
    // case '\', '_', '`': free;
    // case 'a' .. 'h' and 'j' .. 'l': free;
    // case 'n' .. 'r': free;
    // case 't' .. 'w': free;
    // case 'y', 'z': free;
    case 'i': ACC = L0; NEXT;
    case 'm': t1=NR;
            if (t1=='@') { ACC = *(char*)(ACC); }
            else if (t1=='!') { *(char*)expr() = (char)ACC; }
            NEXT;
    case 's': t1=NR; if (t1=='+') { stk[++sp]=(char*)ACC; } 
            else if (t1=='@') { ACC=(cell_t)stk[sp]; }
            else if (t1=='-') { ACC=(cell_t)stk[sp--]; } NEXT;
    case 'x': t1 = NR; if (t1=='T') { ACC = clock(); }
        else if (t1 == 'B') { putchar(' '); }
        else if (t1 == 'N') { putchar(10); }
        else if (t1 == 'U') { LU; }
        else if (t1 == 'H') { ACC = here-(&BYTES(0)); }
        else if (t1 == 'C') { ACC = (cell_t)&BYTES(0); }
        else if (t1 == 'Q') { isBye = 1; }
        NEXT;
    case '{': lsp += 3; L0 = (cell_t)pc; NEXT;
    case '}': if (ACC) { pc = (char*)L0; } else { LU; } NEXT;
    // case '|', '~': free;
    default: printf("-[%d]?-",(int)IR);
    }
}

void Loop() {
    char *y = here;
    int sz = &BYTES(MEM_SZ)-y-1;
    if (input_fp) {
        if (fgets(y, sz, input_fp) != y) {
            fclose(input_fp);
            input_fp = NULL;
        }
    }
    if (!input_fp) {
        // putchar('\n'); putchar('q'); putchar('4'); putchar('>');
        printf("\nq4:(%ld)> ", (long)ACC);
        if (fgets(y, sz, stdin) != y) { isBye=1; return; }
    }
    Run2(y);
}
int main(int argc, char *argv[]) {
    // int r='B';
    // for (i=1; i<argc; ++i) { y=argv[i]; RG[b++] = atoi(y); }
    init();
    // input_fp = fopen("src.q4", "rb");
    while (isBye == 0) { Loop(); }
    return 0;
}
