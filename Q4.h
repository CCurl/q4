// Q4 - a stack VM, inspired by Sandor Schneider's STABLE - https://w3group.de/stable.html

typedef unsigned char byte;
#define MAX_REGS (26*26*26)
#define MAX_CODE (64*1024)
typedef unsigned long addr;
typedef unsigned long ulong;
typedef unsigned short ushort;

typedef struct {
    addr start;
    long from;
    long to;
    addr end;
} LOOP_ENTRY_T;

typedef struct {
    byte   *bmem;
    long   *mem;
    byte   *user;
    ulong   here;
    ulong   user_sz;
    ulong   mem_sz;
    ushort  num_regs;
    ushort  stack_sz;
    long   *dstack;
    addr   *rstack;
    long   dsp, rsp, lsp;
    LOOP_ENTRY_T lstack[4];
} sys_t;

extern sys_t *sys;

#define USER       sys->user
#define REG        sys->mem
#define MEM        sys->mem
#define BMEM       sys->bmem
#define DSTK       sys->dstack
#define RSTK       sys->rstack
#define LSTK       sys->lstack
#define DSP        sys->dsp
#define RSP        sys->rsp
#define LSP        sys->lsp 
#define SZ_USER    sys->user_sz
#define SZ_MEM     sys->mem_sz
#define SZ_STK     sys->stack_sz
#define NUM_REGS   sys->num_regs

extern void vmInit(sys_t *Sys);
extern addr run(addr pc);
extern void dumpStack(int hdr);
extern void setCodeByte(addr loc, char ch);
extern void printString(const char*);
extern void printStringF(const char* fmt, ...);
extern int getChar();
extern int charAvailable();

#ifdef _WIN32
#define __PC__ 1
#define INPUT 0
#define INPUT_PULLUP 1
#define OUTPUT 2
extern long millis();
extern int analogRead(int pin);
extern void analogWrite(int pin, int val);
extern int digitalRead(int pin);
extern void digitalWrite(int pin, int val);
extern void pinMode(int pin, int mode);
extern void delay(unsigned long ms);
extern FILE* input_fp;
extern byte isBye;
#else
#define __PC__ 0
#define _DEV_BOARD_
#define __SERIAL__
#include <Arduino.h>
#define PICO 1
#define XIAO 0
#if PICO
#define CODE_SZ      MAX_CODE   // PICO
#define MEM_SZB      (96*1024)  // PICO
#define NUM_FUNCS    MAX_FUNC   // PICO
#define ILED          25        // PICO
#elif XIAO
#define CODE_SZ      (12*1024)  // XIAO
#define MEM_SZB      (12*1024)  // XIAO
#define NUM_FUNCS    MAX_FUNC   // XIAO
#define ILED          13        // XIAO
#endif
#endif // _WIN32
