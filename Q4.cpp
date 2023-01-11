#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

typedef union {
    char c[4];
    short w[2];
    long l;
} uu_t;

#define CL(n)    code[n].l
#define CW(n,x)  code[n].w[x]
#define CC(n,x)  code[n].c[x]

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

char u, *pc;
long stk[32], rstk[32], lstk[30], reg[256], sp, rsp, lsp, t, isErr;
long locs[100], lb, t1, t2, t3, *pl;
long here, last, vhere;
FILE* input_fp = 0;
uu_t code[CODE_SZ];
dict_t dict[DICT_SZ];
char vars[VARS_SZ];

void vmInit() {
    here = last = vhere = sp = rsp = lsp = 0;
    for (int i=0; i<CODE_SZ; i++) { CL(i) = 0; }
}

int find(char *w) {
    for (int i=1; i<=last; i++) {
        if (strcmp(dict[i].nm, w)==0) { return i; }
    }
    return 0;
}

void create(char *w) {
    if (find(w)) {
        isErr=1;
        printf("-%s already defined-", w);
        return;
    }
    ++last;
    dict[last].xa = here;
    if (16<strlen(w)) { w[16]=0; }
    strcpy(dict[last].nm, w);
}

int gl(int n, long l, char reg) {
    CC(n,0) = 'l';
    CC(n,1) = reg;
    CL(n+1) = l;
    return n+2;
}

int gc(int n, const char *str) {
    CL(n) = 0;
    if (0 < strlen(str)) CC(n,0) = str[0];
    if (1 < strlen(str)) CC(n,1) = str[1];
    if (2 < strlen(str)) CC(n,2) = str[2];
    if (3 < strlen(str)) CC(n,3) = str[3];
    return n + 1;
}

int gs(int n, const char *str) {
    int i = 0;
    while (*str) {
        char c = *(str++);
        if (c == ' ') { ++n; i = 0; continue; }
        if (i < 4) { CC(n,i++) = c; }
    }
    return n + 1;
}

void run(int s) {
    --s;
next:
    switch (CC(++s,0)) {
    case 0: return;
    case '.': if (CC(s,1)==0) { printf("%ld ", PP); }
            else { printf("%ld ", reg[CC(s, 1)]); }
        NEXT;
    case '-': reg[CC(s,1)] = reg[CC(s,2)] - reg[CC(s,3)]; NEXT;
    case '+': reg[CC(s,1)] = reg[CC(s,2)] + reg[CC(s,3)]; NEXT;
    case '/': reg[CC(s,1)] = reg[CC(s,2)] / reg[CC(s,3)]; NEXT;
    case '*': reg[CC(s,1)] = reg[CC(s,2)] * reg[CC(s,3)]; NEXT;
    case '?': if (PP==0) { s = CW(s,1); } NEXT;
    case ':': RPS(s); s = CW(s,1); NEXT;
    case ';': if (rsp) { s = RPP; }
        else { return; }
        NEXT;
    case 'B': printf(" "); NEXT;
    case 'N': printf("\n"); NEXT;
    case 'd': --reg[CC(s,1)]; NEXT;
    case 'i': ++reg[CC(s,1)]; NEXT;
    case 'I': PS(L0); NEXT;
    case 'm': reg[CC(s,1)] = reg[CC(s,2)]; NEXT;
    case 'r': PS(reg[CC(s,1)]); NEXT;
    case 's': reg[CC(s,1)] = PP; NEXT;
    case 'l': if (CC(s,1)) { reg[CC(s, 1)] = CL(s+1); }
            else { PS(CL(s+1));  }
            ++s; NEXT;
    case 't': reg[CC(s,1)] = clock(); NEXT;
    case 'x': if (CC(s,1)=='Q') { exit(0); }
        NEXT;
    case '[': lsp += 3; L0 = PP; L1 = PP; L2 = s; NEXT;
    case ']': if (++L0 < L1) { s = L2; }
            else { lsp -= 3; }
            NEXT;
    default: printf("-ir(%d)?-", CC(s,0));
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
    if (strcmp(w, "reset") == 0) {
        vmInit();
        return l;
    }
    if (strcmp(w, "//") == 0) {
        return 0;
    }
    if (isNum(w)) {
        here = gl(here, PP, 0);
        return l;
    }
    if (strcmp("if",w)==0) {
        PS(here);
        here = gc(here, "?");
        return l;
    }
    if (strcmp("then",w)==0) {
        CW(PP,1) = here;
        return l;
    }
    if (strcmp("fn",w)==0) {
        l = getWord(l, w);
        if (w[0]) { create(w); }
        else { return 0; }
        return l;
    }
    if (strcmp("ret",w)==0) {
        here = gc(here, ";");
        return l;
    }
    int fn = find(w);
    if (fn) {
        short xa = dict[fn].xa;
        CC(here,0) = ':';
        CW(here,1) = xa-1;
        // printf("-%s:%d-\n", w, (int)xa);
        here++;
        return l;
    }
    here = gs(here, w); 
    return l;
}

void compile(char *line) {
    // printf("LINE: %s", line);
    isErr = 0;
    char w[32];
    while (1) {
        line = getWord(line, w);
        if (strlen(w) == 0) { CC(here,0) = 0; return; }
        line = parse(w, line);
        if (line == 0) { return; }
    }
}

void dumpCode() {
    for (int i=0; i<here; i++) {
        printf("%c (%d),%d,%d,%d\n",
            CC(i,0)>31?CC(i,0):2,
            CC(i,0),CC(i,1),CC(i,2),CC(i,3)
        );
    }
}

void runFile() {
    char buf[256];
    while (input_fp) {
        buf[0] = 0;
        if (fgets(buf, sizeof(buf), input_fp) != buf) {
            fclose(input_fp);
            input_fp = 0;
        }
        compile(buf);
        if (isErr) { return; }
    }
    int xa = 0;
    if (last) { xa = dict[last].xa; }
    // dumpCode();
    run(xa);
}

int main() {
    vmInit();
    input_fp = fopen("src.q4", "rb");
    if (input_fp) {
        runFile();
    }
}
