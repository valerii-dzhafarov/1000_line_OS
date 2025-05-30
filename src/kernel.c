#include "kernel.h"
#include "common.h"

extern char __bss[], __bss_end[], __stack_top[];

sbiret sbi_call(long arg0, long arg1, long arg2, long arg3, long arg4,
                       long arg5, long fid, long eid) {

    // register - Указывает компилятору, что переменная должна храниться в регистре
    // __asm__("a0") - Указывает компилятору: положи эту переменную именно в регистр a0
    register long a0 __asm__("a0") = arg0;
    register long a1 __asm__("a1") = arg1;
    register long a2 __asm__("a2") = arg2;
    register long a3 __asm__("a3") = arg3;
    register long a4 __asm__("a4") = arg4;
    register long a5 __asm__("a5") = arg5;
    register long a6 __asm__("a6") = fid;
    register long a7 __asm__("a7") = eid;
    
    //__asm__ — GCC-инструкция для вставки ассемблера
    //__volatile__ — запрещает компилятору удалять или менять порядок вставки
    // "=r"(a0), "=r"(a1) - Выходные операнды — т.е. куда будут записаны результаты после ecall.
    // "r"(a0), ..., "r"(a7) -- Входные операнды — т.е. аргументы, которые нужно передать в ecall.
    //      "r" говорит компилятору: «положи это значение в регистр общего назначения».
    // "memory" - — список побочных эффектов. ecall может изменить память, даже если это не видно. 
    //      Не оптимизируй чтение/запись переменных вокруг него.

    __asm__ __volatile__("ecall"
                         : "=r"(a0), "=r"(a1)
                         : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5),
                           "r"(a6), "r"(a7)
                         : "memory");
    return (sbiret){.error = a0, .value = a1};
}



void putchar(char ch) {
    sbi_call(ch, 0, 0, 0, 0, 0, 0, 1 /* Console Putchar */);
}

void *memset(void *buf, char c, size_t n) {
    uint8_t *p = (uint8_t *) buf;
    while (n--) {
        *(p++) = c;
    }
    return buf;
}

void kernel_main(void) {
    memset(__bss, 0, (size_t) __bss_end - (size_t) __bss);
    printf ("\nHello World! \n %d,%d,%d,%s-\n 0x%x\n%b%" , -128,128,0, "Valery123", 0xf0abcd);
    while (1) {
        __asm__ __volatile__("wfi");
    }

}

__attribute__((section(".text.boot")))
__attribute__((naked))
void boot(void) {
    __asm__ __volatile__(
        "mv sp, %[stack_top]\n" // Set the stack pointer
        "j kernel_main\n"       // Jump to the kernel main function
        :
        : [stack_top] "r" (__stack_top) // Pass the stack top address as %[stack_top]
    );
}