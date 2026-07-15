#ifndef MALLOC_H
#define MALLOC_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define HEAP_SIZE 1024 * 1024 * 64

void heap_init(size_t total_size);

void* kmalloc(size_t size);
void kfree(void* ptr);

size_t get_busy_mem_size();

void *kmemset(void *dest, int value, uint64_t size);
void *kmemcpy(void *dest, void *src, uint64_t size);

#endif
