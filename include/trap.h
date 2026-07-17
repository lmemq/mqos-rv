#include <stdint.h>

#pragma once

void init_traps(void);

void kyield(void);

void kexit(void);

void create_thread(void (*func)(void), uint64_t stack_size);

void ksleep(uint64_t ms);