// q4.cpp - a fast register-based interpreter

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

typedef long cell_t;

#define STK_SZ         32
#define LSTK_SZ        32
#define MEM_SZ     10000
#define CODE_SZ      1000
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

union { cell_t c[MEM_SZ/sizeof(cell_t)]; char b[MEM_SZ]; } mem;

cell_t regs[REGS_SZ], *cell, lstk[LSTK_SZ+1];
char *funcs[FUNCS_SZ];
char *code, *stk[STK_SZ], *pc, *here, isBye;
cell_t acc, sp, lsp, t1, t2;
FILE* input_fp;

void init() {
    code = &mem.b[0];
    cell = &mem.c[0];
    here = &code[0];
    *(here++) = ';';
}

inline float toFlt(int x) { return *(float*)&x; }
inline int toInt(float x) { return *(int*)&x; }

long expr() {
    if (BTW(PC, '0', '9')) {
        t1 = NR - '0';
        while (BTW(PC, '0', '9')) { t1 = (t1 * 10) + NR - '0'; }
        return t1;
    }
    // if (PC=='\'') { ++pc; return NR; } -- TOO SLOW!
    return RG(NR);
}

void XXX() { if (IR) printf("-IR %d (%c)?", IR, IR); pc=0; }
/*<33*/ void NOP() { }
/* ! */ void f33() { t1=NR;
            if (t1=='c') { cell[expr()] = ACC; }
            else if (t1=='b') { code[expr()] = (char)ACC; }
        }
/* " */ void f34() { while (PC!='"') { putchar(NR); } ++pc; }
/* # */ void f35() { }
/* $ */ void f36() { }
/* % */ void f37() { }
/* & */ void f38() { }
/* ' */ void f39() { ACC = NR; }
/* ( */ void f40() { if (!ACC) { while (NR != ')') { ; } } }
/* ) */ void f41() { }
/* * */ void f42() { ACC *= expr(); }
/* + */ void f43() { if (PC == '+') { ++pc; ++RG(NR); } else { ACC += expr(); } }
/* , */ void f44() { putchar((int)ACC); }
/* - */ void f45() { if (PC == '-') { ++pc; --RG(NR); } else { ACC -= expr(); } }
/* . */ void f46() { printf("%ld", (long)ACC); }
/* / */ void f47() { ACC /= expr(); }
/*0-9*/ void n09() { --pc; ACC = expr(); }
/* : */ void f58() { if (PC != ':') { RG(NR) = ACC; return; }
            ++pc; funcs[PC-'A'] = pc+1;
            while (PC) {
                if ((PC==';') && (IR==';')) { break; }
                else { ++pc; }
            } ++pc; here=pc;
        }
/* ; */ void f59() { if (0 < sp) { pc = stk[sp--]; } else { sp = 0; pc = 0; } }
/* < */ void f60() { ACC = (ACC < expr()) ? -1 : 0; }
/* = */ void f61() { ACC = (ACC == expr()) ? -1 : 0;}
/* > */ void f62() { ACC = (ACC > expr()) ? -1 : 0;}
/* ? */ void f63() { }
/* @ */ void f64() { t1=NR;
            if (t1=='c') { ACC = cell[ACC]; }
            else if (t1=='b') { ACC = code[ACC]; }
        }
/*A2Z*/ void A2Z() { ACC = RG(IR); }
/* [ */ void f91() { lsp+=3; L0=0; L1=ACC; L2=(cell_t)pc; }
/* \ */ void f92() { }
/* ] */ void f93() { if (++L0<L1) { pc=(char *)L2; } else { LU; } }
/* ^ */ void f94() { stk[++sp]=pc+1; pc=funcs[PC-'A']; return; }
/* _ */ void f95() { }
/* ` */ void f96() { }
/* i */ void f105() { ACC = L0; }
/* s */ void f115() { t1=NR; if (t1=='+') { stk[++sp]=(char*)ACC; } 
            else if (t1=='@') { ACC=(cell_t)stk[sp]; }
            else if (t1=='-') { ACC=(cell_t)stk[sp--]; } }
/* x */ void f120() { t1 = NR; if (t1=='T') { ACC = clock(); }
            else if (t1 == 'B') { putchar(' '); }
            else if (t1 == 'N') { putchar(10); }
            else if (t1 == 'U') { LU; }
            else if (t1 == 'Q') { isBye = 1; }
        }
/* { */ void f123() { lsp += 3; L0 = (cell_t)pc; }
/* | */ void f124() { }
/* } */ void f125() { if (ACC) { pc = (char*)L0; } else { LU; } }
/* ~ */ void f126() { }

void (*jt[128])()={
    XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  NOP,  NOP,  XXX,  XXX,  NOP,  XXX,  XXX,   //   0 ..  15
    XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,   //  16 ..  31
    NOP,  f33,  f34,  f35,  f36,  f37,  f38,  f39,  f40,  NOP,  f42,  f43,  f44,  f45,  f46,  f47,   //  32 ..  47
    n09,  n09,  n09,  n09,  n09,  n09,  n09,  n09,  n09,  n09,  f58,  f59,  f60,  f61,  f62,  f63,   //  48 ..  63
    f64,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,   //  64 ..  79
    A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  f91,  f92,  f93,  f94,  f95,   //  80 ..  95
    f96,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  f105, XXX,  XXX,  XXX,  XXX,  XXX,  XXX,   //  96 .. 111
    XXX,  XXX,  XXX,  f115, XXX,  XXX,  XXX,  XXX,  f120, XXX,  XXX,  f123, f124, f125, f126, XXX    // 112 .. 127
};

void Run(const char *x) {
    sp = lsp = 0;
    pc = (char *)x;
    while (pc) { jt[NR](); }
}
void Loop() {
    char *y = here;
    int sz = &code[MEM_SZ]-y-1;
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
    Run(y);
}
int main(int argc, char *argv[]) {
    // int r='B';
    // for (i=1; i<argc; ++i) { y=argv[i]; RG[b++] = atoi(y); }
    init();
    input_fp = fopen("src.q4", "rb");
    while (isBye == 0) { Loop(); }
    return 0;
}
