#include <stdint.h>

#pragma once

void init_threads(void);

void yield(void);

void exit(void);

void create_thread(void (*func)(void), uint64_t stack_size);