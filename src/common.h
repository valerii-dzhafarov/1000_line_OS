#pragma once

#define true  1
#define false 0
#define NULL  ((void *) 0)
#define align_up(value, align)   __builtin_align_up(value, align)
#define is_aligned(value, align) __builtin_is_aligned(value, align)
#define offsetof(type, member)   __builtin_offsetof(type, member)
#define va_list  __builtin_va_list
#define va_start __builtin_va_start
#define va_end   __builtin_va_end
#define va_arg   __builtin_va_arg
#define LOG2(x) (31 - __builtin_clz(x))  


#define PAGE_SIZE 4096
#define PAGE_BIT_WIDTH __builtin_ctz(PAGE_SIZE)
#define PAGE_BIT_MASK  (~(0xffffffffffffffff << PAGE_BIT_WIDTH))
#define PAGE_NUMBER (1024 * 16)


typedef int bool;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef uint32_t size_t;


void *memcpy(void *dst, const void *src, size_t n);
void *memset(void *buf, char c, size_t n);
char *strcpy(char *dst, const char *src);
int strcmp(const char *s1, const char *s2);
void printf(const char *fmt, ...);

