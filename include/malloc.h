#ifndef MALLOC_H
#define MALLOC_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

void heap_init(uint64_t start_address, size_t total_size);

void* kmalloc(size_t size);
void kfree(void* ptr);

size_t get_busy_mem_size();

#endif
