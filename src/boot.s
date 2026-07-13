.global _start
_start:
    lla sp, _stack_top # link stack in ram and stack pointer in asm
    jal kernel_main # go to kernel.c
    j . # if kernel returns, like-while(1) in asm
