// S2.c - inspired by Sandor Schneider's STABLE (https://w3group.de/stable.html)

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#define BTW(a,b,c) ((b<=a) && (a<=c))

#define R0 rtos
#define R1 rstk[rsp]

#define STK_SZ         64
#define DATA_SZ      1000
#define CODE_SZ     10000

#define RG(x)        regs[x]
#define ACC          RG('a')
#define IR           *(pc-1)
#define NR           *(pc++)

#define L0           lstk[lsp]
#define L1           lstk[lsp-1]
#define L2           lstk[lsp-2]

char code[CODE_SZ], funcs[26], here;
long data[DATA_SZ];
long regs[128], lstk[100];
FILE* input_fp;

static char ex[256], *pc;
static char *y, *stk[STK_SZ];
static int sp, lsp, t1, t2, isBye;

inline float toFlt(int x) { return *(float*)&x; }
inline int toInt(float x) { return *(int*)&x; }

long expr() {
    if (BTW(*pc, '0', '9')) {
        t1 = *(pc++) - '0';
        while (BTW(*pc, '0', '9')) { t1 = (t1 * 10) + *(pc++) - '0'; }
        return t1;
    }
    return RG(NR);
}

void XXX() { if (IR && (IR!=10)) printf("-IR %d (%c)?", IR, IR); pc=0; }
/*<33*/ void NOP() { }
/* ! */ void f33() { data[expr()] = ACC; }
/* " */ void f34() { while (*pc!='"') { putchar(*(pc++)); } ++pc; }
/* # */ void f35() { }
/* $ */ void f36() { }
/* & */ void f37() { }
/* & */ void f38() { }
/* ' */ void f39() { ACC = NR; }
/* ( */ void f40() { if (!ACC) { while (*(pc++) != ')') { ; } } }
/* ) */ void f41() { }
/* * */ void f42() { ACC *= expr(); }
/* + */ void f43() { if (*pc == '+') { ++pc; ++RG(NR); } else { ACC += expr(); } }
/* , */ void f44() { putchar((int)ACC); }
/* - */ void f45() { if (*pc == '-') { ++pc; ++RG(NR); } else { ACC -= expr(); } }
/* . */ void f46() { printf("%ld", ACC); }
/* / */ void f47() { ACC /= expr(); }
/*0-9*/ void n09() { --pc; ACC = expr(); }
/* : */ void f58() { if (*pc != ':') { RG(NR) = ACC; return; }
            ++pc; funcs[NR-'A'] = here; while (*pc) {
                if ((*pc==';') && (code[here-1]==';')) { break; }
                else { code[here++]=*(pc++); }
            } ++pc;
        }
/* ; */ void f59() { if (0 < sp) { pc = stk[sp--]; } else { sp = 0; pc = 0; } }
/* < */ void f60() { ACC = (ACC < expr()) ? -1 : 0; }
/* = */ void f61() { ACC = (ACC == expr()) ? -1 : 0;}
/* > */ void f62() { ACC = (ACC > expr()) ? -1 : 0;}
/* ? */ void f63() { }
/* @ */ void f64() { ACC = data[ACC]; }
/*A2Z*/ void A2Z() { ACC = RG(IR); }
/* [ */ void f91() { lsp+=3; L0=0; L1=ACC; L2=(long)pc; }
/* \ */ void f92() { }
/* ] */ void f93() { if (++L0<L1) { pc=(char *)L2; } else { lsp-=3; } }
/* ^ */ void f94() { stk[++sp]=pc+1; pc=&code[funcs[*pc-'A']]; return; }
/* _ */ void f95() { }
/* ` */ void f96() { }
/* d */ void f100() { }
/* i */ void f105() { ACC = L0; }
/*a-z*/ void a2z() { }
/* x */ void f120() { t1 = NR; if (t1=='T') { ACC = clock(); }
            else if (t1 == 'B') { putchar(' '); }
            else if (t1 == 'N') { putchar(10); }
            else if (t1 == 'U') { lsp =- 3; }
            else if (t1 == 'Q') { isBye = 1; }
        }
/* { */ void f123() { lsp += 3; L0 = (long)pc; }
/* | */ void f124() { }
/* } */ void f125() { if (ACC) { pc = (char*)L0; } else { lsp -= 3; } }
/* ~ */ void f126() { }

void (*jt[128])()={
    XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  NOP,  NOP,  XXX,  XXX,  NOP,  XXX,  XXX,   //   0 ..  15
    XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,   //  16 ..  31
    NOP,  f33,  f34,  f35,  f36,  f37,  f38,  f39,  f40,  NOP,  f42,  f43,  f44,  f45,  f46,  f47,   //  32 ..  47
    n09,  n09,  n09,  n09,  n09,  n09,  n09,  n09,  n09,  n09,  f58,  f59,  f60,  f61,  f62,  f63,   //  48 ..  63
    f64,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,   //  64 ..  79
    A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  f91,  f92,  f93,  f94,  f95,   //  80 ..  95
    f96,  a2z,  a2z,  a2z,  f100, a2z,  a2z,  a2z,  a2z,  f105, a2z,  a2z,  a2z,  a2z,  a2z,  a2z,   //  96 .. 111
    a2z,  a2z,  a2z,  a2z,  a2z,  a2z,  a2z,  a2z,  f120, a2z,  a2z,  f123, f124, f125, f126, XXX    // 112 .. 127
};

void Run(const char *x) {
    sp = lsp = 0;
    pc = (char *)x;
    while (pc) { jt[*(pc++)](); }
}
void H(char *s) { /* FILE* fp = fopen("h.txt", "at"); if (fp) { fprintf(fp, "%s", s); fclose(fp); } */ }
void Loop() {
    char buf[256] = { 0 };
    if (input_fp) {
        if (fgets(buf, 128, input_fp) != buf) {
            fclose(input_fp);
            input_fp = NULL;
        }
        // if (input_fp) { printf("%s",buf); }
    }
    if (!input_fp) {
        printf("\nq4:(%d)> ", ACC);
        if (fgets(buf, 128, stdin) != buf) { *buf='\\'; *(buf+1)='q'; }
    }
    // H(ex);
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
