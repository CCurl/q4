#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

typedef union {
    char c[4];
    short w[2];
    long l[1];
} uu_t;

#define CL(n,x)  code[n].l[x]
#define CW(n,x)  code[n].w[x]
#define CC(n,x)  code[n].c[x]
#define PL(x)    p->l[x]
#define PW(x)    p->w[x]
#define PC(x)    p->c[x]

typedef struct {
    char nm[16];
    short xa;
} dict_t;

#define CODE_SZ   10000
#define DICT_SZ    1000
#define VARS_SZ    1000*1000

#define NEXT     goto next
#define PS(x)    stk[++sp]=(x)
#define PP       stk[sp--]
#define TOS      stk[sp]
#define NOS      stk[sp-1]
#define RPS(x)   rstk[++rsp]=(x)
#define RPP      rstk[rsp--]
#define R0       rstk[rsp]
#define R1       rstk[rsp-1]
#define R2       rstk[rsp-2]
#define D1       sp--
#define D2       sp-=2
#define L0       lstk[lsp]
#define L1       lstk[lsp-1]
#define L2       lstk[lsp-2]
#define L3       lstk[lsp-3]
#define BTW(a,b,c) ((b<=a)&&(a<=c))

#define LIT    1
#define CALL   2
#define RET    3

char u, *pc;
long stk[32], rstk[32], lstk[30], reg[256], sp, rsp, lsp, t, isErr;
long locs[100], lb, t1, t2, t3, *pl;
long here, last, vhere;
uu_t code[CODE_SZ];
dict_t dict[DICT_SZ];
char vars[VARS_SZ];

void compileFile(FILE* fp);

void vmInit() {
    here = last = vhere = sp = rsp = lsp = 0;
    for (int i=0; i<CODE_SZ; i++) { CL(i,0) = 0; }
}

void setErr(const char *msg) { printf("ERROR: %s\n", msg); isErr=1; }

int find(char *w) {
    for (int i=1; i<=last; i++) {
        if (strcmp(dict[i].nm, w)==0) { return i; }
    }
    return 0;
}

void create(char *w) {
    if (find(w)) { printf("-redef: %s-", w); }
    ++last;
    dict[last].xa = (short)here;
    if (15<strlen(w)) { w[16]=0; }
    strcpy(dict[last].nm, w);
}

int glit(int n, long l, char reg) {
    CC(n,0) = LIT;
    CC(n,1) = reg;
    CL(n+1,0) = l;
    return n+2;
}

int gcode(int n, const char *str) {
    CL(n,0) = 0;
    if (0 < strlen(str)) CC(n,0) = str[0];
    if (1 < strlen(str)) CC(n,1) = str[1];
    if (2 < strlen(str)) CC(n,2) = str[2];
    if (3 < strlen(str)) CC(n,3) = str[3];
    return n + 1;
}

int gstr(int n, const char *str) {
    int i = 0;
    while (*str) {
        char c = *(str++);
        if (c == ' ') { ++n; i = 0; continue; }
        if (i < 4) { CC(n,i++) = c; }
    }
    return n + 1;
}

void run(int s) {
next:
    uu_t* p = &code[s++];
    // printf("-pc:%d:(%d)-",s-1,PC(0));
    switch (PC(0)) {
    case 0: return;
    case LIT: if (PC(1)) { reg[PC(1)]=CL(s++,0); }
        else { PS(CL(s++,0)); }; NEXT;
    case CALL: RPS(0); RPS(s); s = PW(1); NEXT;
    case RET: if (rsp<1) { rsp=0; return; }
        s = RPP; t1 = RPP; if (t1) {
            lb -= t1; if (lsp<0) { lsp=0; }
            for (int i=0; i<t1; i++) { reg['0'+i]=locs[lb+i]; }
        } NEXT;
    case '.': if (PC(1)) { printf("%ld", reg[PC(1)]); }
        else { printf("%ld", PP); }
        if (PC(2)=='B') { printf(" "); }
        NEXT;
    case ',': if (PC(1)) { printf("%c", (char)reg[PC(1)]); }
        else { printf("%c", (char)PP); }
        NEXT;
    case '-': reg[PC(1)] = reg[PC(2)] - reg[PC(3)]; NEXT;
    case '+': reg[PC(1)] = reg[PC(2)] + reg[PC(3)]; NEXT;
    case '/': reg[PC(1)] = reg[PC(2)] / reg[PC(3)]; NEXT;
    case '*': reg[PC(1)] = reg[PC(2)] * reg[PC(3)]; NEXT;
    case '\'': PS(PC(1)); NEXT;
    case '=': NOS=(NOS==TOS)?-1:0; D1; NEXT;
    case '<': NOS=(NOS<TOS)?-1:0; D1; NEXT;
    case '>': NOS=(NOS>TOS)?-1:0; D1; NEXT;
    case '?': if (PP==0) { s = PW(1); } NEXT;
    case 'B': printf(" "); NEXT;
    case 'N': printf("\n"); NEXT;
    case 'd': --reg[PC(1)]; NEXT;
    case 'i': ++reg[PC(1)]; NEXT;
    case 'I': PS(L0); NEXT;
    case 'l': if (PC(1)=='+') {
            R1 = PP+1;
            for (int i=0; i<R1; i++) { locs[lb+i]=reg['0'+i]; }
            lb += R1;
        } NEXT;
    case 'm': reg[PC(1)] = reg[PC(2)]; NEXT;
    case 'r': PS(reg[PC(1)]); NEXT;
    case 's': reg[PC(1)] = PP; NEXT;
    case 't': reg[PC(1)] = clock(); NEXT;
    case 'x': if (PC(1)=='Q') { exit(0); }
        NEXT;
    case '[': lsp += 3; L0 = PP; L1 = PP; L2 = s; NEXT;
    case ']': if (++L0 < L1) { s = L2; }
            else { lsp -= 3; }
            NEXT;
    case '}': if (PC(1)=='r') { s = PW(1); }
            else if ((PC(1)=='w') && (TOS)) { s = PW(1); }
            else if ((PC(1)=='u') && (!TOS)) { s = PW(1); }
            D1;  NEXT;
    default: printf("-ir(%d)?-", PC(0));
    }
}

char *getWord(char *line, char *w) {
    while ((*line) && (*line <= ' ')) { ++line; }
    while (' ' < *line) { *(w++) = *(line++); }
    *w = 0;
    return line;
}

int isNum(char* w) {
    int n = 0;
    while (*w) {
        if (!BTW(*w, '0', '9')) { return 0; }
        n = (n*10) + (*(w++)-'0');
    }
    PS(n);
    return 1;
}

char *parse(char* w, char* l) {
    // printf("(%s)", w);
    if (strcmp(w, "//") == 0) { return 0; }
    if (strcmp(w, "reset") == 0) { vmInit(); return l; }
    if (isNum(w)) { here = glit(here, PP, 0); return l; }
    if ((w[0]=='s') && (CC(here-2,0)==LIT)) { CC(here-2,1)=w[1]; return l; }

    if (strcmp("if",w)==0) { PS(here); here=gcode(here, "?"); return l; }
    if (strcmp("then",w)==0) { CW(PP,1)=(short)here; return l; }

    #define WUR(a) here=gcode(here,a); CW(here-1,1)=(short)PP; return l
    if (strcmp("begin",w)==0) { PS(here); return l; }
    if (strcmp("while",w)==0) { WUR("}w"); }
    if (strcmp("until",w)==0) { WUR("}u"); }
    if (strcmp("repeat",w)==0) { WUR("}r"); }

    if (strcmp("fn",w)==0) {
        l = getWord(l, w);
        if (w[0]) { create(w); }
        else { return 0; }
        return l;
    }
    if (strcmp("load",w)==0) {
        l = getWord(l, w);
        FILE* fp = fopen(w, "rb");
        if (fp) { compileFile(fp); fclose(fp); }
        else { setErr("cannot open file (load)"); }
        return isErr ? 0 : l;
    }
    t1 = find(w);
    if (t1) {
        short xa = dict[t1].xa;
        CC(here,0) = CALL;
        CW(here++,1) = xa;
        return l;
    }
    if (strcmp("ret",w)==0) {
        CW(here++,0) = RET;
        return l;
    }
    here = gstr(here, w); 
    return l;
}

void compile(char *line) {
    // printf("LINE: %s", line);
    char w[32];
    isErr = 0;
    while (1) {
        line = getWord(line, w);
        if (strlen(w) == 0) { CC(here,0) = 0; return; }
        line = parse(w, line);
        if (line == 0) { return; }
    }
}

#define MP(x) (x>31)?x:'.'
void dumpCode() {
    for (int i=0; i<here; i++) {
        int a=CC(i,0), b=CC(i,1), c=CC(i,2), d=CC(i,3);
        printf("%3i: %4d, %4d, %4d, %4d - %c%c%c%c\n",
            i, a, b, c, d, MP(a), MP(b), MP(c), MP(d)
        );
    }
}

void compileFile(FILE *fp) {
    char buf[256];
    while (fp) {
        buf[0] = 0;
        if (fgets(buf, sizeof(buf), fp) != buf) { return; }
        compile(buf);
        if (isErr) { return; }
    }
}

void runFile(const char *fn) {
    char buf[32];
    sprintf(buf, "load %s", fn);
    compile(buf);
    if (isErr == 0) {
        // dumpCode();
        int xa = (last) ? dict[last].xa : 0;
        run(xa);
    }
}

int main() {
    vmInit();
    runFile("src.q4");
}
