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
#define ACC          RG('A')
#define IR           *(pc-1)
#define NR           *(pc++)

#define L0           lstk[lsp]
#define L1           lstk[lsp-1]
#define L2           lstk[lsp-2]

char code[CODE_SZ], funcs[26], here;
long data[DATA_SZ];
long regs[128], lstk[100];

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
/* # */ void f35() { stk[++sp] = pc+1; pc = &code[funcs[*pc-'A']]; }
/* $ */ void f36() { }
/* % */ void f37() { funcs[NR-'A'] = here; while (*pc) {
                if ((*pc==';') && (code[here-1]==';')) { break; }
                else { code[here++]=*(pc++); }
            } ++pc;
        }
/* & */ void f38() { }
/* ' */ void f39() { ACC = NR; }
/* ( */ void f40() { if (!ACC) { while (*(pc++) != ')') { ; } } }
/* ) */ void f41() { }
/* * */ void f42() { ACC *= expr(); }
/* + */ void f43() { ACC += expr(); }
/* , */ void f44() { putchar((int)ACC); }
/* - */ void f45() { ACC -= expr(); }
/* . */ void f46() { printf("%ld", ACC); }
/* / */ void f47() { ACC /= expr(); }
/*0-9*/ void n09() { --pc; ACC = expr(); }
/* : */ void f58() { RG(NR) = ACC; }
/* ; */ void f59() { if (0 < sp) { pc = stk[sp--]; } else { sp = 0; pc = 0; } }
/* < */ void f60() { ACC = (ACC < expr()) ? -1 : 0; }
/* = */ void f61() { ACC = (ACC == expr()) ? -1 : 0;}
/* > */ void f62() { ACC = (ACC > expr()) ? -1 : 0;}
/* ? */ void f63() { }
/* @ */ void f64() { ACC = data[ACC]; }
/*A2Z*/ void A2Z() { ACC = RG(IR); }
/* [ */ void f91() { lsp+=3; L0=0; L1=ACC; L2=(long)pc; }
/* \ */ void f92() { t1 = NR; if (t1=='t') { ACC = clock(); }
            else if (t1 == 'i') { ACC = L0; }
            else if (t1 == 'u') { lsp =- 3; }
            else if (t1 == 'q') { isBye = 1; }
        }
/* ] */ void f93() { if (++L0<L1) { pc=(char *)L2; } else { lsp-=3; } }
/* ^ */ void f94() { ++RG(NR); }
/* _ */ void f95() { --RG(NR); }
/* ` */ void f96() { }
/*a-z*/ void a2z() { t1 = IR; printf("-call (%c)-", t1); }
/* { */ void f123() { lsp += 3; L0 = (long)pc; }
/* | */ void f124() { }
/* } */ void f125() { if (ACC) { pc = (char*)L0; } else { lsp -= 3; } }
/* ~ */ void f126() { }

void (*jt[128])()={
    XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,   //   0 ..  15
    XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,   //  16 ..  31
    NOP,  f33,  f34,  f35,  f36,  f37,  f38,  f39,  f40,  NOP,  f42,  f43,  f44,  f45,  f46,  f47,   //  32 ..  47
    n09,  n09,  n09,  n09,  n09,  n09,  n09,  n09,  n09,  n09,  f58,  f59,  f60,  f61,  f62,  f63,   //  48 ..  63
    f64,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,   //  64 ..  79
    A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  f91,  f92,  f93,  f94,  f95,   //  80 ..  95
    f96,  a2z,  a2z,  a2z,  a2z,  a2z,  a2z,  a2z,  a2z,  a2z,  a2z,  a2z,  a2z,  a2z,  a2z,  a2z,   //  96 .. 111
    a2z,  a2z,  a2z,  a2z,  a2z,  a2z,  a2z,  a2z,  a2z,  a2z,  a2z,  f123, f124, f125, f126, XXX    // 112 .. 127
};

void Run(const char *x) {
    sp = lsp = 0;
    pc = (char *)x;
    while (pc) { jt[*(pc++)](); }
}
void H(char *s) { /* FILE* fp = fopen("h.txt", "at"); if (fp) { fprintf(fp, "%s", s); fclose(fp); } */ }
void Loop() {
    printf("\nq4: ");
    if (fgets(ex, 128, stdin) != ex) { ex[0] = '~'; }
    // H(ex);
    Run(ex);
}
int main(int argc, char *argv[]) {
    //int i,j; s=sb-1; h=cb; ir=SZ-500; for (i=0; i<(SZ/4); i++) { st.i[i]=0; }
    //st.i[0]=h; st.i[lb]=argc; for (i=1; i < argc; ++i) { y=argv[i]; t=atoi(y);
    //    if ((t) || (y[0]=='0' && y[1]==0)) { st.i[lb+i]=t; }
    //    else { st.i[lb+i]=ir; for (j=0; y[j]; j++) { st.b[ir++]=y[j]; } st.b[ir++]=0; } }
    //if ((argc>1) && (argv[1][0]!='-')) { FILE *fp=fopen(argv[1], "rb"); 
    //    if (fp) {while ((c=fgetc(fp))!=EOF) { st.b[h++]=(31<c)?c:32; } fclose(fp); st.i[0]=h; R(cb); }
    //} 
    here = 0; code[here++] = ';';
    Run("%B32,;; %N10,;;");
    Run("123:B 321+B:C B. #B C. #N");
    Run("33:B ;BC C. #N");
    Run("200:B 111!B B@:C B. #B C. #N");
    while (isBye == 0) { Loop(); }
    return 0;
}
