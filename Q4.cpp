#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

typedef union {
    char c[4];
    long l;
} uu_t;

#define NEXT     goto next
#define CL(n)    code[n].l
#define CC(n,x)  code[n].c[x]
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
long stk[32], rstk[32], lstk[30], reg[256], sp, rsp, lsp, t, here, isErr;
long locs[100], lb, t1, t2, t3, *pl;
FILE* input_fp = 0;
uu_t code[10000];

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
    while (*line == ' ') { ++line; }
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

int parse(char* w) {
    if (strcmp(w, "reset") == 0) { here = 0; }
    if (isNum(w)) { here = gl(here, PP, 0); return 1; }
    else { here = gs(here, w); return 1; }
    printf("[%s]??", w);
    isErr = 1;
    return 0;
}

void compile(char *line) {
    isErr = 0;
    char w[32];
    while (1) {
        line = getWord(line, w);
        if (strlen(w) == 0) { CC(here,0) = 0; return; }
        if (parse(w)) { continue; }
        return;
    }
}

void loop() {
    int xx = here;
    char buf[256];
    if (input_fp != stdin) {
        if (fgets(buf, sizeof(buf), input_fp) != buf) {
            fclose(input_fp);
            input_fp = stdin;
        }
    }
    if (input_fp == stdin) {
        printf("\nq4: ");
        fgets(buf, sizeof(buf), input_fp);
    } else { printf("%s", buf); }
    compile(buf);
    if (!isErr) { run(xx); }
}

int main() {
    sp = rsp = lsp = lb = 0;
    input_fp = fopen("src.q4", "rb");
    here = 0;
    while (1) { loop(); }
}
