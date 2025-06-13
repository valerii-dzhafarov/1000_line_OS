#include "kernel.h"
#include "common.h"

extern char __bss[], __bss_end[];
extern char __stack_top[];
extern size_t __ram_meta[], __ram_meta_end[];
extern char __free_ram[], __free_ram_end[];

sbiret sbi_call(long arg0, long arg1, long arg2, long arg3, long arg4, long arg5, long fid,
                long eid) {

    // register - Указывает компилятору, что переменная должна храниться в
    // регистре
    // __asm__("a0") - Указывает компилятору: положи эту переменную именно в
    // регистр a0
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
    // "=r"(a0), "=r"(a1) - Выходные операнды — т.е. куда будут записаны
    // результаты после ecall. "r"(a0), ..., "r"(a7) -- Входные операнды — т.е.
    // аргументы, которые нужно передать в ecall.
    //      "r" говорит компилятору: «положи это значение в регистр общего
    //      назначения».
    // "memory" - — список побочных эффектов. ecall может изменить память, даже
    // если это не видно.
    //      Не оптимизируй чтение/запись переменных вокруг него.

    __asm__ __volatile__("ecall"
                         : "=r"(a0), "=r"(a1)
                         : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a6), "r"(a7)
                         : "memory");
    return (sbiret){.error = a0, .value = a1};
}

void putchar(char ch) {
    sbi_call(ch, 0, 0, 0, 0, 0, 0, 1 /* Console Putchar */);
}

process_s procs[PROCS_MAX_WITH_IDLE];    // +1 for idle process

void delay(void) {
    for (int i = 0; i < 30000000; i++)
        __asm__ __volatile__("nop");    // do nothing
}

process_s* proc_a;
process_s* proc_b;

process_s* curr_proc;    // Currently running process
process_s* idle_proc;    // Idle process

void proc_b_entry(void) {
    printf("starting process B\n");
    int i = 3;
    while (i--) {
        printf("B: sp: 0x%x\n", curr_proc->sp);
        printf("B : i: %d\n", i);

        yield();
        delay();
    }
    curr_proc->state = PROC_IDLE;    // Mark as idle

}

void proc_a_entry(void) {
    printf("starting process A\n");
    //        putchar('A');
    int i = 3;
    while (i--) {
        printf("A: sp: 0x%x\n", curr_proc->sp);
        printf("A : i: %d\n", i);
        yield();
        delay();
    }
    //delay();
    curr_proc->state = PROC_IDLE;    // Mark as idle
}

__attribute__((naked)) void switch_context(size_t* curr_sp, size_t* next_sp) {
    __asm__ __volatile__(
        "addi sp, sp, -4 * 13\n"
        "sw ra, 0 * 4 (sp) \n"
        "sw s0,  1 * 4 (sp)  \n"
        "sw s1,  2 * 4 (sp)  \n"
        "sw s2,  3 * 4 (sp)  \n"
        "sw s3,  4 * 4 (sp)  \n"
        "sw s4,  5 * 4 (sp)  \n"
        "sw s5,  6 * 4 (sp)  \n"
        "sw s6,  7 * 4 (sp)  \n"
        "sw s7,  8 * 4 (sp)  \n"
        "sw s8,  9 * 4 (sp)  \n"
        "sw s9,  10 * 4 (sp) \n"
        "sw s10, 11 * 4 (sp) \n"
        "sw s11, 12 * 4 (sp) \n"

        "sw sp, (a0) \n"
        "lw sp, (a1) \n"

        "lw ra,  0 * 4 (sp) \n"
        "lw s0,  1 * 4 (sp)  \n"
        "lw s1,  2 * 4 (sp)  \n"
        "lw s2,  3 * 4 (sp)  \n"
        "lw s3,  4 * 4 (sp)  \n"
        "lw s4,  5 * 4 (sp)  \n"
        "lw s5,  6 * 4 (sp)  \n"
        "lw s6,  7 * 4 (sp)  \n"
        "lw s7,  8 * 4 (sp)  \n"
        "lw s8,  9 * 4 (sp)  \n"
        "lw s9,  10 * 4 (sp)  \n"
        "lw s10, 11 * 4 (sp) \n"
        "lw s11, 12 * 4 (sp) \n"

        "addi sp, sp, 4 * 13 \n"
        "ret\n");
}

void yield(void) {

    // Search for a runnable process
    process_s* next = idle_proc;
    for (int i = 1; i < PROCS_MAX_WITH_IDLE; i++) {
        process_s* proc = &procs[(curr_proc->pid + i) % PROCS_MAX_WITH_IDLE];
        if (proc->state == PROC_RUNNABLE && (proc->pid > 0)) {
            next = proc;
            break;
        }
    }

    if (next == idle_proc)
        printf("switching to idle process\n");

    // If there's no runnable process other than the current one, return and continue processing
    if (next == curr_proc) {
        return;
    }
    
    // Context switch
    process_s* prev = curr_proc;
    curr_proc = next;
    switch_context(&prev->sp, &next->sp);

}


void process_wrapper () {
    
}

process_s* create_process(size_t pc) {

    process_s* proc = NULL;
    size_t index;

    for (index = 0; index < PROCS_MAX_WITH_IDLE; ++index) {
        if (procs[index].state == PROC_IDLE) {
            proc = &(procs[index]);
            break;
        }
    }

    if (!proc) {
        PANIC("No empty process slots");
    }

    uint8_t* stack_bottom = proc->stack + sizeof(proc->stack);
    size_t* sp = (size_t*)stack_bottom;

    for (int i = 0; i < 12; ++i) {
        *--sp = 0;    // s0-s11 to zero. array can be unrolled. but it is OS for dummies
    }
    *--sp = (size_t)pc;    // ra - return address

    // Initialize fields.
    proc->pid = index;
    proc->state = PROC_RUNNABLE;
    proc->sp = (size_t)sp;

    printf("\nProcess created: proc = 0x%x sp = 0x%x, pid = 0x%x,  state = 0x%x\n", proc, proc->sp,
           proc->pid, proc->state);
    return proc;
}

void kernel_main(void) {
    memset(__bss, 0, (size_t)__bss_end - (size_t)__bss);
    memset(__ram_meta, 0, (size_t)__ram_meta_end - (size_t)__ram_meta);
    WRITE_CSR(stvec, (uint32_t)kernel_entry);

    idle_proc = create_process((uint32_t)NULL);
    idle_proc->pid = 0;    // idle
    curr_proc = idle_proc;

    proc_a = create_process((uint32_t)proc_a_entry);
    proc_b = create_process((uint32_t)proc_b_entry);

    yield();
    PANIC("switched to idle process");
}

__attribute__((section(".text.boot"))) __attribute__((naked)) void boot(void) {
    __asm__ __volatile__(
        "mv sp, %[stack_top]\n"    // Set the stack pointer
        "j kernel_main\n"          // Jump to the kernel main function
        :
        : [stack_top] "r"(__stack_top)    // Pass the stack top address as %[stack_top]
    );
}

__attribute__((naked)) __attribute__((aligned(4))) void kernel_entry(void) {
    __asm__ __volatile__(
        "csrw sscratch, sp\n"
        "addi sp, sp, -4 * 31\n"
        "sw ra,  4 * 0(sp)\n"
        "sw gp,  4 * 1(sp)\n"
        "sw tp,  4 * 2(sp)\n"
        "sw t0,  4 * 3(sp)\n"
        "sw t1,  4 * 4(sp)\n"
        "sw t2,  4 * 5(sp)\n"
        "sw t3,  4 * 6(sp)\n"
        "sw t4,  4 * 7(sp)\n"
        "sw t5,  4 * 8(sp)\n"
        "sw t6,  4 * 9(sp)\n"
        "sw a0,  4 * 10(sp)\n"
        "sw a1,  4 * 11(sp)\n"
        "sw a2,  4 * 12(sp)\n"
        "sw a3,  4 * 13(sp)\n"
        "sw a4,  4 * 14(sp)\n"
        "sw a5,  4 * 15(sp)\n"
        "sw a6,  4 * 16(sp)\n"
        "sw a7,  4 * 17(sp)\n"
        "sw s0,  4 * 18(sp)\n"
        "sw s1,  4 * 19(sp)\n"
        "sw s2,  4 * 20(sp)\n"
        "sw s3,  4 * 21(sp)\n"
        "sw s4,  4 * 22(sp)\n"
        "sw s5,  4 * 23(sp)\n"
        "sw s6,  4 * 24(sp)\n"
        "sw s7,  4 * 25(sp)\n"
        "sw s8,  4 * 26(sp)\n"
        "sw s9,  4 * 27(sp)\n"
        "sw s10, 4 * 28(sp)\n"
        "sw s11, 4 * 29(sp)\n"

        "csrr a0, sscratch\n"
        "sw a0, 4 * 30(sp)\n"

        "mv a0, sp\n"
        "call handle_trap\n"

        "lw ra,  4 * 0(sp)\n"
        "lw gp,  4 * 1(sp)\n"
        "lw tp,  4 * 2(sp)\n"
        "lw t0,  4 * 3(sp)\n"
        "lw t1,  4 * 4(sp)\n"
        "lw t2,  4 * 5(sp)\n"
        "lw t3,  4 * 6(sp)\n"
        "lw t4,  4 * 7(sp)\n"
        "lw t5,  4 * 8(sp)\n"
        "lw t6,  4 * 9(sp)\n"
        "lw a0,  4 * 10(sp)\n"
        "lw a1,  4 * 11(sp)\n"
        "lw a2,  4 * 12(sp)\n"
        "lw a3,  4 * 13(sp)\n"
        "lw a4,  4 * 14(sp)\n"
        "lw a5,  4 * 15(sp)\n"
        "lw a6,  4 * 16(sp)\n"
        "lw a7,  4 * 17(sp)\n"
        "lw s0,  4 * 18(sp)\n"
        "lw s1,  4 * 19(sp)\n"
        "lw s2,  4 * 20(sp)\n"
        "lw s3,  4 * 21(sp)\n"
        "lw s4,  4 * 22(sp)\n"
        "lw s5,  4 * 23(sp)\n"
        "lw s6,  4 * 24(sp)\n"
        "lw s7,  4 * 25(sp)\n"
        "lw s8,  4 * 26(sp)\n"
        "lw s9,  4 * 27(sp)\n"
        "lw s10, 4 * 28(sp)\n"
        "lw s11, 4 * 29(sp)\n"
        "lw sp,  4 * 30(sp)\n"
        "sret\n");
}

void handle_trap(struct trap_frame* f) {
    (void)f;
    uint32_t scause = READ_CSR(scause);
    uint32_t stval = READ_CSR(stval);
    uint32_t sepc = READ_CSR(sepc);
    PANIC("unexpected trap scause=%x, stval=%x, sepc=%x\n", scause, stval, sepc);
}

static inline uint32_t* find_empty_meta_pages(size_t pages_num) {

    printf("pages_num = %d\n", pages_num);
    uint32_t* ram_meta_start_pos = NULL;
    size_t found_pages_cnt = 0;

    for (uint32_t* i = __ram_meta; i < __ram_meta_end;) {
        uint32_t status = (*i) & PAGE_BIT_MASK;
        if (status == 1) {
            ram_meta_start_pos = NULL;
            found_pages_cnt = 0;
            i += ((*i) >> PAGE_BIT_WIDTH);
        } else {
            if (ram_meta_start_pos == NULL) {
                ram_meta_start_pos = i;
            }
            found_pages_cnt++;
            if (found_pages_cnt == pages_num) {
                return ram_meta_start_pos;
            }
            i++;
        }
    }

    return NULL;
}

size_t alloc_pages(size_t size) {

    if (!size) {
        PANIC("Alloc with zero size\n");
        return 0;
    }

    size_t req_pages = (size >> PAGE_BIT_WIDTH) + ((size & PAGE_BIT_MASK) != 0);
    uint32_t* meta_addr = find_empty_meta_pages(req_pages);

    if (meta_addr == NULL) {
        PANIC("No available empty mem for size = %d\n", size);
        return 0;
    }

    *meta_addr = 1 | (req_pages << PAGE_BIT_WIDTH);
    size_t page_ordered_num = (meta_addr - __ram_meta);
    return (size_t)((page_ordered_num << PAGE_BIT_WIDTH) + __free_ram);
}

void free_pages(size_t addr) {
    size_t meta_num = ((char*)addr - __free_ram) >> PAGE_BIT_WIDTH;
    uint32_t* meta_addr = __ram_meta + meta_num;
    *meta_addr = 0;
}
