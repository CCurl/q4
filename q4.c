// q4.cpp - a fast register-based interpreter

// Windows PC (Visual Studio)?
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
#define CODE_SZ          2048
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
#define LU           lsp=(lsp<3)?0:(lsp-3)

#define BYTES(x)      mem.b[x]
#define CELLS(x)      mem.c[x]

typedef long cell_t;
union { cell_t c[MEM_SZ/sizeof(cell_t)]; char b[MEM_SZ]; } mem;

cell_t regs[REGS_SZ], lstk[LSTK_SZ+1];
char *funcs[FUNCS_SZ], *stk[STK_SZ], *pc, *here;
cell_t acc, sp, lsp, t1;

#ifdef isPC
FILE *input_fp;
int isBye;
#endif

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
    if (PC == '.') { ++pc; return ACC; }
    return RG(NR);
    // if ('A' <= PC) { return RG(NR); }
    // return ACC;
}

#define NEXT goto next
void Run(const char *x) {
    sp = lsp = 0;
    pc = (char *)x;
next:
    switch (NR) {
    case 0: return;
    case 9: case 10: case 13: case ' ': NEXT;
    case '!': t1=NR;
            if (t1=='c') { CELLS(expr()) = ACC; }
            else if (t1=='b') { BYTES(expr()) = (char)ACC; }
            else if (t1=='m') { *(char*)(expr()) = (char)ACC; }
        NEXT;
    case '"': while (PC && (PC!='"')) { putchar(NR); } if (PC) ++pc; NEXT;
    case '%': ACC %= expr(); NEXT;
    // '#' .. '&' are free;
    case '\'': ACC = NR; NEXT;
    case '(': if (!ACC) { while (NR != ')') { ; } } NEXT;
    case ')': NEXT;
    case '*': ACC *= expr(); NEXT;
    case '+': ACC += expr(); NEXT;
    case ',': putchar((int)expr()); NEXT;
    case '-': ACC -= expr(); NEXT;
    case '.': t1=NR; if (t1 == 'b') { putchar(' '); }
        else if (t1 == 'n') { putchar(10); }
        else if (t1 == 'h') { printf("%lx", (long)expr()); }
        else { --pc; printf("%ld", expr()); }
        NEXT;
    case '/': ACC /= expr(); NEXT;
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9': --pc; ACC = expr(); NEXT;
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
    // case '?': free;
    case '@': t1=NR;
            if (t1=='c') { ACC = CELLS(expr()); }
            else if (t1=='b') { ACC = BYTES(expr()); }
            else if (t1=='m') { ACC = *(char*)(expr()); }
            NEXT;
    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
    case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
    case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
    case 'V': case 'W': case 'X': case 'Y': case 'Z': ACC = RG(IR);  NEXT;
    case '[': lsp+=3; L0=RG('I'); RG('I')=0; L1=ACC; L2=(cell_t)pc; NEXT;
    case ']': if (++RG('I')<L1) { pc=(char *)L2; } else { RG('I')=L0; LU; } NEXT;
    case '^': stk[++sp]=pc+1; pc=funcs[PC-'A']; NEXT;
    // case '\', '_', '`' are free;
#ifdef isPC
    case '`': { char *x=here+64, *y=x; while ( PC!='`') { *(y++)=NR; }
            *y=0; ACC=system(x); }; NR; NEXT;
#endif
    case 'd': --RG(NR); NEXT;
    case 'i': ++RG(NR); NEXT;
    case 'm': t1 = NR; RG(NR) = RG(t1); NEXT;
    case 'r': t1=NR; if (t1=='.') { ACC=(cell_t)stk[+sp--]; } 
            else if (t1=='@') { ACC=(cell_t)stk[sp]; }
            else { RG(t1)=(cell_t)stk[sp--]; }
            NEXT;
    case 's': t1=NR; if (t1=='.') { stk[++sp]=(char*)ACC; } 
            else { stk[++sp]=(char*)RG(t1); } 
            NEXT;
    case 'x': t1 = NR; if (t1=='T') { ACC = clock(); }
        else if (t1 == 'U') { LU; }
        else if (t1 == 'H') { ACC = here-(&BYTES(0)); }
        else if (t1 == 'M') { ACC = (cell_t)&BYTES(0); }
#ifdef isPC
        else if (t1 == 'Q') { isBye = 1; }
#endif
        NEXT;
    case '{': lsp += 3; L0 = (cell_t)pc; NEXT;
    case '}': if (ACC) { pc = (char*)L0; } else { LU; } NEXT;
    // case '|', '~': free;
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
        printf("\nq4:(%ld)> ", (long)ACC);
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
