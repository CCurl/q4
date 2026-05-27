// q4.cpp - a register-based interpreter

// Windows PC (Visual Studio)
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#define isPC
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
//#include <math.h>

#define STK_SZ             32
#define LSTK_SZ            11
#define MEM_SZ          10000
#define CODE_SZ         20480
#define REGS_SZ           128 // 'z'-'A'+1
#define FUNCS_SZ         1023

#define BTW(a,b,c) ((b<=a) && (a<=c))

// #define ACC          regs[dstReg]
#define RG(x)        regs[(x)]
// #define RG(x)        regs[(x)-'A']

#define PC           *(pc)
#define IR           *(pc-1)
#define NR           *(pc++)

#define L0           lstk[lsp]
#define L1           lstk[lsp-1]
#define L2           lstk[lsp-2]

#define BYTES(x)      mem.b[x]
#define CELLS(x)      mem.c[x]

typedef long cell_t;
union { cell_t c[MEM_SZ/sizeof(cell_t)]; char b[MEM_SZ]; } mem;

cell_t regs[REGS_SZ], lstk[LSTK_SZ+1];
char *funcs[FUNCS_SZ+1], *stk[STK_SZ], *pc, *here, *fa, dstReg;
cell_t acc, lcc, sp, lsp, t1, fn;

#ifdef isPC
FILE *input_fp;
int isBye;
#endif

void init() {
    here = &BYTES(0);
    *(here++) = ';';
    for (int i = 0;i <= FUNCS_SZ; i++) { funcs[i] = 0; }
}

char *funcN(char *x) {
    cell_t hh = *(x++);
    while (BTW(*x, 'A', 'Z')) { hh = (hh * 33) + *(x++); }
    fn = (hh & FUNCS_SZ); fa = funcs[fn];
    return x;
}

// inline float toFlt(int x) { return *(float*)&x; }
// inline int toInt(float x) { return *(int*)&x; }

long expr() {
    if (BTW(PC, '0', '9')) {
        t1 = NR - '0';
        while (BTW(PC, '0', '9')) { t1 = (t1 * 10) + NR - '0'; }
        return t1;
    }
    if (PC == '.') { ++pc; return acc; }
    return RG(NR);
    // if ('A' <= PC) { return RG(NR); }
    // return ACC;
}

int unLoop(int ret) {
    if (2<lsp) { RG('I')=L0; lsp-=3; }
    if (ret && (0<sp)) { pc=stk[sp--]; return (pc)?1:0; }
    return 0;
}

#define NEXT goto next
void Run(const char *x) {
    sp = lsp = 0;
    pc = (char *)x;
next:
    switch (NR) {
    case 0: case 9: case 10: case 13: case ' ':
        if (dstReg) { RG(dstReg)=acc; dstReg=0; } else { lcc = acc; }
        if (IR) { NEXT; } else { return; }
    case '!': t1=NR;
            if (t1=='c') { CELLS(expr()) = acc; }
            else if (t1=='b') { BYTES(expr()) = (char)acc; }
            else if (t1=='m') { *(char*)(expr()) = (char)acc; }
        NEXT;
    case '"': while (PC && (PC!='"')) { putchar(NR); } if (PC) ++pc; NEXT;
    case '#': t1=expr(); RG('R')=acc%t1; RG('Q')=acc/t1; acc=RG('R'); NEXT;
    // case '$': NEXT;
    case '%': acc %= expr(); NEXT;
    case '&': acc &= expr(); NEXT;
    case '\'': acc = NR; NEXT;
    case '(': if (!acc) { while (NR != ')') { ; } } NEXT;
    case ')': NEXT;
    case '*': acc *= expr(); NEXT;
    case '+': acc += expr(); NEXT;
    case ',': putchar((int)expr()); NEXT;
    case '-': acc -= expr(); NEXT;
    case '.': t1=NR; if (t1 == 'b') { putchar(' '); }
        else if (t1 == 'n') { putchar(10); }
        else if (t1 == 'h') { printf("%lx", (long)expr()); }
        else if (t1 == '"') { while (PC && (PC!='"')) { putchar(NR); } if (PC) ++pc; }
        else { --pc; printf("%ld", expr()); }
        NEXT;
    case '/': acc /= expr(); NEXT;
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
            --pc; acc = expr(); NEXT;
    case ':': if (PC != ':') { pc=funcN(pc); if (fa) { stk[++sp]=pc; pc=fa; } NEXT; }
            pc = funcN(pc+1); if (fa) { printf("-redef at %ld-", fa-&BYTES(0)); }
            while (PC == ' ') { ++pc; }
            funcs[fn] = pc;
            while (PC) {
                if ((IR == ';') && (PC == ';')) { here = ++pc; NEXT; }
                else { ++pc; }
            }
            printf("-no ';;'-");
    case ';': if (0 < sp) { pc = stk[sp--]; } else { return; } NEXT;
    case '<': acc = (acc < expr()) ? -1 : 0; NEXT;
    case '=': acc = (acc == expr()) ? -1 : 0; NEXT;
    case '>': acc = (acc > expr()) ? -1 : 0; NEXT;
    // case '?': free;
    case '@': t1=NR;
            if (t1=='c') { acc = CELLS(expr()); }
            else if (t1=='b') { acc = BYTES(expr()); }
            else if (t1=='m') { acc = *(char*)(expr()); }
            NEXT;
    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
    case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
    case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
    case 'V': case 'W': case 'X': case 'Y': case 'Z': 
            if (PC == ':') { dstReg=IR; ++pc; }
            else { acc = RG(IR); }
            NEXT;
    case '[': lsp+=3; L0=RG('I'); RG('I')=acc; L1=lcc; L2=(cell_t)pc; NEXT;
    case ']': if (++RG('I')<L1) { pc=(char *)L2; } else { unLoop(0); } NEXT;
    case '^': ; if (unLoop(1)) { NEXT; } else { return; }
    // case '\', '_', '`' are free;
#ifdef isPC
    case '`': { char *x=here+64, *y=x; while ( PC!='`') { *(y++)=NR; }
            *y=0; acc=system(x); }; ++pc; NEXT;
#endif
    case 'd': --RG(NR); NEXT;
    case 'i': ++RG(NR); NEXT;
    case 'm': t1 = NR; RG(NR) = RG(t1); NEXT;
    case 'r': t1=NR; if (t1=='.') { acc=(cell_t)stk[sp--]; } 
            else if (t1=='@') { acc=(cell_t)stk[sp]; }
            else { RG(t1)=(cell_t)stk[sp--]; }
            NEXT;
    case 's': t1=NR; if (t1=='.') { stk[++sp]=(char*)acc; } 
            else { stk[++sp]=(char*)RG(t1); } 
            NEXT;
    case 'x': t1 = NR;
        if (t1 == 'T') { acc = clock(); }
        else if (t1 == 'H') { acc = here-(&BYTES(0)); }
        else if (t1 == 'M') { acc = (cell_t)&BYTES(0); }
#ifdef isPC
        else if (t1 == 'Q') { isBye = 1; }
#endif
        NEXT;
    case '{': lsp += 3; L2 = (cell_t)pc; L0=RG('I'); NEXT;
    case '|': acc |= expr(); NEXT;
    case '}': if (acc) { pc = (char*)L2; } else { unLoop(0); } NEXT;
    case '~': acc = acc ? 0 : -1; NEXT;
    default: printf("-[%d]?-",(int)IR);
    }
}

#ifdef isPC
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
        printf("\nq4:(%ld)> ", acc);
        if (fgets(y, sz, stdin) != y) { isBye=1; return; }
    }
    Run(y);
}
int main(int argc, char *argv[]) {
    // int r='A';
    // for (i=1; i<argc; ++i) { y=argv[i]; RG(r++) = atoi(y); }
    init();
    input_fp = fopen("src.q4", "rb");
    while (isBye == 0) { Loop(); }
    return 0;
}
#endif
