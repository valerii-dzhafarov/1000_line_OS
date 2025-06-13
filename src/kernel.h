#pragma once

#include "common.h"

#define PANIC(fmt, ...)                                                        \
  do {                                                                         \
    printf("PANIC: %s:%d: " fmt " \n", __FILE__, __LINE__, ##__VA_ARGS__);     \
    while (1) {                                                                \
    }                                                                          \
  } while (0)

#define PROCS_MAX 8 // Maximum number of processes
#define PROCS_MAX_WITH_IDLE (PROCS_MAX+1)

typedef enum { PROC_IDLE = 0, PROC_RUNNABLE } process_state_e;

typedef struct {
  int pid;               // Process ID
  process_state_e state; // Process state
  size_t  sp;            // Stack pointer
   __attribute__((aligned(4))) uint8_t stack[8192];   // Kernel stack 
} process_s;

typedef struct {
  long error;
  long value;
} sbiret;

struct trap_frame {
  uint32_t ra;
  uint32_t gp;
  uint32_t tp;
  uint32_t t0;
  uint32_t t1;
  uint32_t t2;
  uint32_t t3;
  uint32_t t4;
  uint32_t t5;
  uint32_t t6;
  uint32_t a0;
  uint32_t a1;
  uint32_t a2;
  uint32_t a3;
  uint32_t a4;
  uint32_t a5;
  uint32_t a6;
  uint32_t a7;
  uint32_t s0;
  uint32_t s1;
  uint32_t s2;
  uint32_t s3;
  uint32_t s4;
  uint32_t s5;
  uint32_t s6;
  uint32_t s7;
  uint32_t s8;
  uint32_t s9;
  uint32_t s10;
  uint32_t s11;
  uint32_t sp;
} __attribute__((packed));

#define READ_CSR(reg)                                                          \
  ({                                                                           \
    unsigned long __tmp;                                                       \
    __asm__ __volatile__("csrr %0, " #reg : "=r"(__tmp));                      \
    __tmp;                                                                     \
  })

#define WRITE_CSR(reg, value)                                                  \
  do {                                                                         \
    uint32_t __tmp = (value);                                                  \
    __asm__ __volatile__("csrw " #reg ", %0" ::"r"(__tmp));                    \
  } while (0)

void kernel_entry(void);

size_t alloc_pages(size_t size);
void free_pages(size_t addr);
void yield(void);

process_s *create_process(uint32_t pc);
void switch_context(size_t* , size_t* );