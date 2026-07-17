#include <stddef.h>

#include "malloc.h"
#include "trap.h"
#include "serial.h"

#define MAX_THREADS 64
#define TIME_QUANTUM 100000
#define TICKS_IN_MS 10000
#define MS_IN_SYS_TICK 10 

__attribute__((noinline)) uint64_t get_kernel_gp(void) {
    uint64_t gp_val;
    asm volatile("mv %0, gp" : "=r"(gp_val));
    return gp_val;
}

typedef enum {
    TR_SOFTWARE = 1, // one core pings another
    TR_TIMER = 7, 
    TR_EXTERNAL = 9, // kb, mouse, etc.
} Hardware;

typedef enum {
    EX_INSTRUCTION_ADDR_MISALIGNED = 0, // addr % 2, 4 != 0
    EX_INSTRUCTION_ADDR_FAULT = 1, // mem is not accessible
    EX_ILLEGAL_INSTRUCTION = 2, // garbage instead of command
    INST_BREAKPOINT = 3, // ebreak in asm
    EX_LOAD_ADDR_MISALIGNED = 4, // falied to load not aligned address
    EX_LOAD_ADDR_FAULT = 5, // falied to load: mem not accessible
    EX_STORE_ADDR_MISALIGNED = 6, // falied to store in not aligned addr
    EX_STORE_ADDR_FAULT = 7, // cant write: mem protected / read only
    INST_USER_CALL = 8, // ecall from userspace
    INST_SYS_CALL = 9, // call from system, deprecated (is wrong for m-mode now, it s from s-mode)
    INST_M_MODE_CALL = 11, // ecall 
    EX_INSTRUCTION_PAGE_FAULT = 12, // x: page not in ram
    EX_LOAD_PAGE_FAULT = 13, // r: page not in ram
    EX_STORE_PAGE_FAULT = 15 // w: page not in ram 
} Software; // inst is instruction, tr - trap, ex - exception

typedef enum {
    STATUS_DEAD = 0,
    STATUS_ALIVE = 1,
    STATUS_SLEEP = 2
} Status;

typedef struct thread_data {
    uint64_t ra;  // x1
    uint64_t sp;  // x2
    uint64_t gp;  // x3
    uint64_t tp;  // x4
    uint64_t t0, t1, t2; // x5 - x7
    uint64_t s0, s1; // x8 - x9
    uint64_t a0, a1, a2, a3, a4, a5, a6, a7; // x10 - x17
    uint64_t s2, s3, s4, s5, s6, s7, s8, s9, s10, s11; // x18 - x27
    uint64_t t3, t4, t5, t6; // x28 - x31
    uint64_t sepc; // pc
    uint64_t scause;  // trap cause
    uint64_t stack_size;
    uint64_t sleep_sys_ticks;
    char status; 
} thread_data;

extern void _trap_vector(void);

int8_t current_i = 0;
uint64_t system_ticks = 0;

thread_data threads[MAX_THREADS]; 

uint64_t get_time(void) {
    return *(volatile uint64_t*)0x200BFF8;
}

void set_timer(uint64_t next_tick) {
    *(volatile uint64_t*)0x2004000 = next_tick;
}

void scheduler_tick(void) {
    int8_t old_i = current_i;
    current_i = (old_i + 1) % MAX_THREADS;
    while (threads[current_i].status != STATUS_ALIVE) {
        current_i = (current_i + 1) % MAX_THREADS; 
    }
    asm volatile("csrw mscratch, %0" : : "r"(&threads[current_i]));
}

void init_traps(void) {
    kmemset(threads, 0, MAX_THREADS * sizeof(thread_data));
    threads[0].status = STATUS_ALIVE; 
    uint64_t current_gp;
    asm volatile("mv %0, gp" : "=r"(current_gp));
    threads[0].gp = current_gp;  
    asm volatile("csrw mtvec, %0" : : "r"(_trap_vector)); // trap controller address
    asm volatile("csrw mscratch, %0" : : "r"(&threads[0])); // address of first thread to sscratch
    set_timer(get_time() + TIME_QUANTUM);
    asm volatile("csrs mie, %0" : : "r"(1ULL << 7)); // allow traps of timer
    asm volatile("csrs mstatus, %0" : : "r"(1ULL << 3)); // allow traps 
}

thread_data* handle_trap(thread_data* tf) {
    uint64_t cause = tf->scause;
    uint64_t interrupt_code = cause & 0xFFF; 
    if (cause & (1ULL << 63)) { // 1ull - 1 uns llong. 1000.....00000 (63 zeros)
        switch (interrupt_code) { // hardware traps
            case TR_TIMER:
                uint64_t mie; asm volatile("csrr %0, mie" : "=r"(mie)); mie &= ~(1ULL << 7); asm volatile("csrw mie, %0" : : "r"(mie));
                system_ticks++;
                set_timer(get_time() + TIME_QUANTUM);
                for (int8_t i = 0; i < MAX_THREADS; i++) {
                    if (threads[i].status == STATUS_SLEEP) {
                        threads[i].sleep_sys_ticks--;
                        if (threads[i].sleep_sys_ticks == 0) {
                            threads[i].status = STATUS_ALIVE;
                        }
                    }
                }
                scheduler_tick();
                asm volatile("csrs mie, %0" : : "r"(1ULL << 7));
                break;
            
            default:
                
                break;
        }
    }
    else {
        switch (interrupt_code) { // software traps
            case INST_M_MODE_CALL:
                tf->sepc += 4; 
                scheduler_tick();
                break;
            
            case EX_ILLEGAL_INSTRUCTION:
                kprint("KERNEL PANIC: Illegal Instruction at 0x");
                kprint_ui(tf->sepc);
                kprint("\n");
                while(1);

            default:
                kprint("KERNEL PANIC: Unhandled Exception ");
                kprint_ui(cause);
                kprint(" ");
                kprint_ui(tf->sepc);
                kprint("\n");
                while(1);
        }
    }
    return &threads[current_i];
}

void kyield(void) {
    asm volatile("ecall");
}

void kexit(void) {
    threads[current_i].status = STATUS_DEAD;
    kyield();
}

void ksleep(uint64_t ms) {
    kprint_ui(ms);
    kprint("\n");
    uint64_t ticks_to_sleep = ms / MS_IN_SYS_TICK;
    if (ticks_to_sleep == 0) ticks_to_sleep = 1; 
    threads[current_i].sleep_sys_ticks = ticks_to_sleep;
    threads[current_i].status = STATUS_SLEEP;
    kyield();
}

void create_thread(void (*func)(void), uint64_t stack_size) {
    if (stack_size == 0 || stack_size >= 1024 * 1024) {
        return;
    }
    uint8_t current_i_ = 0, checked = 0;
    while (threads[current_i_].status != STATUS_DEAD) {
        current_i_ = (current_i_ + 1) % MAX_THREADS; 
        checked++;
        if (checked >= MAX_THREADS) {
            return;
        }
    }
    if (threads[current_i_].sp != 0) {  
        kfree((void*)(threads[current_i_].sp - threads[current_i_].stack_size));
    }
    uint64_t stack_top = (uint64_t)kmalloc(stack_size) + stack_size;
    kmemset(&threads[current_i_], 0, sizeof(thread_data));
    threads[current_i_].sepc = (uint64_t)func;
    threads[current_i_].sp = stack_top;
    threads[current_i_].stack_size = stack_size;
    threads[current_i_].status = STATUS_ALIVE;
    threads[current_i_].sleep_sys_ticks = 0;
    threads[current_i_].gp = get_kernel_gp();
    threads[current_i_].a0 = 0; 
}
