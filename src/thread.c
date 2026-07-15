#include <stddef.h>

#include "malloc.h"
#include "thread.h"

#define MAX_THREADS 64

typedef struct thread_data {
    uint64_t ra, sp, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, stack_size;
    char alive;
} thread_data;

extern void _switch(thread_data* old, thread_data* new);

int8_t current_i = 0;

thread_data threads[MAX_THREADS]; 

void init_threads(void) {
    // threads = kmalloc(MAX_THREADS * sizeof(thread_data));
    kmemset(threads, 0, MAX_THREADS * sizeof(thread_data));
    threads[0].alive = 1; 
}

void yield(void) {
    int8_t old_i = current_i;
    current_i = (old_i + 1) % MAX_THREADS;
    while (!threads[current_i].alive) {
        current_i = (current_i + 1) % MAX_THREADS; 
    }
    _switch(&threads[old_i], &threads[current_i]);
}

void exit(void) {
    threads[current_i].alive = 0;
    yield();
}

void create_thread(void (*func)(void), uint64_t stack_size) {
    if (stack_size == 0 || stack_size >= 1024 * 1024) {
        return;
    }
    uint8_t current_i_ = 0, checked = 0;
    while (threads[current_i_].alive) {
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
    threads[current_i_].ra = (uint64_t)func;
    threads[current_i_].sp = stack_top;
    threads[current_i_].stack_size = stack_size;
    threads[current_i_].alive = 1;
}