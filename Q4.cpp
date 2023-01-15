// Q4.cpp - a fast register-based interpreter

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

typedef long cell_t;

#define STK_SZ         32
#define LSTK_SZ        32
#define DATA_SZ     10000
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

cell_t regs[REGS_SZ], data[DATA_SZ], funcs[FUNCS_SZ], lstk[LSTK_SZ+1];
char code[CODE_SZ], *stk[STK_SZ], *pc, ex[32], isBye;
cell_t acc, here, sp, lsp, t1, t2;
FILE* input_fp;

inline float toFlt(cell_t x) { return *(float*)&x; }
inline cell_t toCell(float x) { return *(cell_t*)&x; }

long expr() {
    if (BTW(PC, '0', '9')) {
        t1 = NR - '0';
        while (BTW(PC, '0', '9')) { t1 = (t1 * 10) + NR - '0'; }
        if (PC=='e') { t1=toCell((float)t1); ++pc; }
        return t1;
    }
    // if (PC=='\'') { ++pc; return NR; } -- TOO SLOW!
    return RG(NR);
}

void XXX() { if (IR) printf("-IR %d (%c)?", IR, IR); pc=0; }
/*<33*/ void NOP() { }
/* ! */ void f33() { data[expr()] = ACC; }
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
            ++pc; funcs[NR-'A'] = here; while (PC) {
                if ((PC==';') && (code[here-1]==';')) { break; }
                else { code[here++]=NR; }
            } ++pc;
        }
/* ; */ void f59() { if (0 < sp) { pc = stk[sp--]; } else { sp = 0; pc = 0; } }
/* < */ void f60() { ACC = (ACC < expr()) ? -1 : 0; }
/* = */ void f61() { ACC = (ACC == expr()) ? -1 : 0;}
/* > */ void f62() { ACC = (ACC > expr()) ? -1 : 0;}
/* ? */ void f63() { }
/* @ */ void f64() { ACC = data[ACC]; }
/*A2Z*/ void A2Z() { ACC = RG(IR); }
/* [ */ void f91() { lsp+=3; L0=0; L1=ACC; L2=(cell_t)pc; }
/* \ */ void f92() { }
/* ] */ void f93() { if (++L0<L1) { pc=(char *)L2; } else { LU; } }
/* ^ */ void f94() { stk[++sp]=pc+1; pc=&code[funcs[PC-'A']]; return; }
/* _ */ void f95() { }
/* ` */ void f96() { }
/* f */ void f102() { float f1, f2; t1 = NR;
            if (t1=='f') { ACC=toCell((float)ACC); }
            if (t1=='i') { ACC=(cell_t)toFlt(ACC); }
            if (t1=='.') { printf("%g", toFlt(ACC)); }
            if (t1=='+') { ACC = toCell(toFlt(ACC) + toFlt(expr())); }
            if (t1=='-') { ACC = toCell(toFlt(ACC) - toFlt(expr())); }
            if (t1=='*') { ACC = toCell(toFlt(ACC) * toFlt(expr())); }
            if (t1=='/') { ACC = toCell(toFlt(ACC) / toFlt(expr())); }
            if (t1=='<') { ACC = (cell_t)(toFlt(ACC) < toFlt(expr())) ? -1 : 0; }
            if (t1=='>') { ACC = (cell_t)(toFlt(ACC) > toFlt(expr())) ? -1 : 0; }
            // if (t1=='O') { ;;; }
            // if (t1=='C') { ;;; }
            // if (t1=='R') { ;;; }
            // if (t1=='W') { ;;; }
            // if (t1=='@') { ;;; }
            // if (t1=='@') { ;;; }
        }
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
    f96,  XXX,  XXX,  XXX,  XXX,  XXX,  f102, XXX,  XXX,  f105, XXX,  XXX,  XXX,  XXX,  XXX,  XXX,   //  96 .. 111
    XXX,  XXX,  XXX,  f115, XXX,  XXX,  XXX,  XXX,  f120, XXX,  XXX,  f123, f124, f125, f126, XXX    // 112 .. 127
};

void Run(const char *x) {
    sp = lsp = 0;
    pc = (char *)x;
    while (pc) { jt[NR](); }
}
void Loop() {
    char buf[128] = { 0 };
    if (input_fp) {
        if (fgets(buf, sizeof(buf), input_fp) != buf) {
            fclose(input_fp);
            input_fp = NULL;
        }
    }
    if (!input_fp) {
        // putchar('\n'); putchar('q'); putchar('4'); putchar('>');
        printf("\nq4:(%ld)> ", (long)ACC);
        if (fgets(buf, sizeof(buf), stdin) != buf) { isBye=1; return; }
    }
    Run(buf);
}
int main(int argc, char *argv[]) {
    // int r='B';
    // for (i=1; i<argc; ++i) { y=argv[i]; RG[b++] = atoi(y); }
    here = 1; code[0] = ';';
    input_fp = fopen("src.q4", "rb");
    while (isBye == 0) { Loop(); }
    return 0;
}
