#include <stdint.h>

#pragma once

void init_traps(void);

void yield(void);

void exit(void);

void create_thread(void (*func)(void), uint64_t stack_size);

void sleep(uint64_t ms);