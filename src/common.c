#include "common.h"



void putchar(char ch);

void printf(const char* fmt, ...) {
    va_list vargs;
    va_start(vargs, fmt);
    
    while (*fmt) {

        if ((*fmt) == '%') {
            ++fmt;
            switch (*fmt) {
            case 's': {
                const char* s = va_arg(vargs, const char*);
                while (*s) {
                    putchar(*s++);
                    //++s;
                }
                break;
            }
            case 'd': {
                int d = va_arg(vargs, int);
                unsigned m = d;
                if (d<0) {
                    m = d * -1;
                    putchar('-');
                }
                if (d==0) {
                    putchar('0');
                }
                else {
                    char num [10];
                    int cnt = 1;
                    while (m) { 
                        num[cnt] = m % 10 + '0';
                        m /= 10; 
                        cnt++;
                    }
                    while (cnt) {
                        putchar(num[cnt-1]);
                        cnt --;
                    }
                }
                break;
            }
            case 'x': { // Print an integer in hexadecimal.
                    unsigned value = va_arg(vargs, unsigned);
                    if (!value) {
                        putchar('0');
                        break;
                    }
                    unsigned non_zero_started = 0; 
                    for (int i = 7; i >= 0; i--) {
                        unsigned nibble = (value >> (i * 4)) & 0xf;
                        if ((nibble == 0) && (non_zero_started == 0))
                            continue;

                        non_zero_started = 1;
                        putchar("0123456789abcdef"[nibble]);
                    }
                    break;
                }
            default:
                putchar('%');
                fmt--;
            }
        } else {
            putchar(*fmt);
        }

        ++fmt;
    }
    va_end(vargs);
}


void* memcpy(void *dst, const void *src, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        *((char*)dst + i) = *((const char*)src + i);
    }
    return dst;
}

void* memset(void *buf, char c, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        *((char*)buf + i) = c;
    }
    return buf;

}

char *strcpy(char *dst, const char *src) {
    char *d = dst;
    while (*src)
        *d++ = *src++;
    *d = '\0';
    return dst;
}

char *strcpy_s(char *dst, size_t dstsize, const char *src) {
    size_t cnt = 0;
    while (true) {
        if (dstsize == cnt++)
            break;
        *(dst++) = *(src++);
        if (*src == '\0') break; 
    }

    return dst;
}

int strcmp(const char *s1, const char *s2) {

    while (*s1 && *s2 ) {
        if (*s1 != *s2) {
            break;
        }
        ++s1;
        ++s2;
    }

    return *(unsigned char*)s1 - *(unsigned char*)s2;
}





