#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include "Q4.h"

typedef unsigned char byte;

#define REGS_NUM   MAX_REGS
#define CODE_SZ    (32*1024)
#define REGS_SZ    (REGS_NUM*4)
#define VARS_SZ    (64*1024)
#define STK_SZ           31
#define MEM_SZ     REGS_SZ+CODE_SZ+VARS_SZ

static byte memory[MEM_SZ];
static long dstk[STK_SZ + 1];
static addr rstk[STK_SZ + 1];
static sys_t mySys;
static long tibOffset = CODE_SZ - 256;
static byte *tib = NULL;

// These are used only by the PC version
static HANDLE hStdOut = 0;
static char input_fn[32];
static char runMode = 'i';

// These are in the <Arduino.h> file
long millis() { return GetTickCount(); }
int analogRead(int pin) { printStringF("-AR(%d)-", pin); return 0; }
void analogWrite(int pin, int val) { printStringF("-AW(%d,%d)-", pin, val); }
int digitalRead(int pin) { printStringF("-DR(%d)-", pin); return 0; }
void digitalWrite(int pin, int val) { printStringF("-DW(%d,%d)-", pin, val); }
void pinMode(int pin, int mode) { printStringF("-pinMode(%d,%d)-", pin, mode); }
void delay(DWORD ms) { Sleep(ms); }

void printString(const char* str) {
    DWORD n = 0, l = (DWORD)strlen(str);
    if (l) { WriteConsoleA(hStdOut, str, l, &n, 0); }
}

int getChar() {
    return _getch();
}

int charAvailable() {
    return _kbhit();
}

void ok() {
    printString("\r\nQ4:"); dumpStack(0); printString(">");
}

void doHistory(const char* txt) {
    FILE* fp = NULL;
    fopen_s(&fp, "history.txt", "at");
    if (fp) {
        fprintf(fp, "%s", txt);
        fclose(fp);
    }
}

addr strToCode(addr loc, const char *txt, int NT) {
    char c = *(txt++);
    while (c) {
        if (c == 9) { c = ' '; }
        if (' ' <= c) { setCodeByte(loc++, c); }
        c = *(txt++);
    }
    if (NT) { setCodeByte(loc++, 0); }
    return loc;
}

void loop() {
    char *tib = (char*)&mySys.code[tibOffset];
    FILE* fp = (input_fp) ? input_fp : stdin;
    if (fp == stdin) { ok(); }
    if (fgets(tib, 128, fp) == tib) {
        if (fp == stdin) { doHistory(tib); }
        run(tibOffset);
        return;
    }
    if (input_fp) {
        fclose(input_fp);
        input_fp = NULL;
        if (runMode == 'n') { isBye = 1; }
    } else { 
        isBye = 1; 
    }
}

void process_arg(char* arg)
{
    if (*arg == 'n') { runMode = 'n'; }
    else if (*arg == 'i') { runMode = 'i'; }
    else if ((*arg == 's') && (*(arg + 1) == ':')) {
        arg = arg + 2;
        strcpy_s(input_fn, sizeof(input_fn), arg);
    }
    else if (*arg == '?') {
        printString("usage Q4 [arguments] [code]\n");
        printString("  -s:[source-filename]\n");
        printString("  -i  Run in interactive mode (default)\n");
        printString("  -n  Run in non-interactive mode\n");
    }
    else { printf("unknown arg '-%s' (try Q4 -?)\n", arg); }
}

sys_t *createSystem() {
    mySys.bmem = memory;
    mySys.code_sz = CODE_SZ;
    mySys.mem_sz = MEM_SZ;
    mySys.num_regs = REGS_NUM;
    mySys.dstack = dstk;
    mySys.rstack = rstk;
    mySys.stack_sz = STK_SZ;
    return &mySys;
}

int main(int argc, char** argv) {
    hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD m; GetConsoleMode(hStdOut, &m);
    SetConsoleMode(hStdOut, (m | ENABLE_VIRTUAL_TERMINAL_PROCESSING));

    vmInit(createSystem());

    input_fn[0] = 0;
    input_fp = NULL;

    for (int i = 1; i < argc; i++)
    {
        char* cp = argv[i];
        if (*cp == '-') { process_arg(++cp); }
        else { 
            strToCode(tibOffset, cp, 1);
            run(tibOffset);
        }
    }

    if (strlen(input_fn) > 0) {
        input_fp = fopen(input_fn, "rt");
    }

    if ((!input_fp) && (runMode == 'n')) { isBye = 1; }
    while (isBye == 0) { loop(); }
}

#endif // #define _WIN32
