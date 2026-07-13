ENTRY(_start)
SECTIONS {
    . = 0x80000000;
    .text : { *(.text) }
    .data : { *(.data) }
    .bss : { *(.bss COMMON) }
    . = ALIGN(8);
    . += 0x10000; /* 64 kb for stack */
    _stack_top = .;
}
