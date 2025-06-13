ENTRY(boot)

SECTIONS {
    . = 0x80200000;

    .text :{ /* instructions */
        KEEP(*(.text.boot));
        *(.text .text.*);
    }

    .rodata : ALIGN(4) {
        *(.rodata .rodata.*);
    }

    .data : ALIGN(4) {
        *(.data .data.*);
    }

    .bss : ALIGN(4) {
        __bss = .;
        *(.bss .bss.* .sbss .sbss.*);
        __bss_end = .;
    }

    . = ALIGN(4);
    . += 128 * 1024; /* 128KB */
    __stack_top = .;

    . = ALIGN(4);
    __ram_meta = .;
    . += 4 * (1024 * 16); /* 16K words*/
    __ram_meta_end = .;

    . = ALIGN(4096);
    __free_ram = .;
    . += 4096 * (1024 * 16); /* 64MB = 16K pages*/ 
    __free_ram_end = .;
    
}