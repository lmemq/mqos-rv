#include "malloc.h"

typedef struct BlockHeader {
    size_t size;
    bool is_free;
    struct BlockHeader* next;
    struct BlockHeader* prev;
} BlockHeader;

#define HEADER_SIZE sizeof(BlockHeader)
#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

static BlockHeader* free_list_head = NULL;

static size_t busy_mem_size = 0;

void heap_init(uint64_t start_address, size_t total_size) {
    uint64_t aligned_start = ALIGN(start_address);
    long long remaining_size = total_size - (aligned_start - start_address);

    if (remaining_size <= (long long)HEADER_SIZE) return;

    free_list_head = (BlockHeader*)aligned_start;
    free_list_head->size = remaining_size - HEADER_SIZE;
    free_list_head->is_free = true;
    free_list_head->next = NULL;
    free_list_head->prev = NULL;
}

void* kmalloc(size_t size) {
    if (size == 0) return NULL;

    size = ALIGN(size);

    BlockHeader* current = free_list_head;

    while (current != NULL) {
        if (current->is_free && current->size >= size) {
            if (current->size >= size + HEADER_SIZE + ALIGNMENT) {
                BlockHeader* next_block = (BlockHeader*)((uint8_t*)current + HEADER_SIZE + size);
                
                next_block->size = current->size - size - HEADER_SIZE;
                next_block->is_free = true;
                next_block->next = current->next;
                next_block->prev = current;

                if (current->next != NULL) {
                    current->next->prev = next_block;
                }

                current->size = size;
                current->next = next_block;
            }
            current->is_free = false;

            busy_mem_size += current->size;

            return (void*)((uint8_t*)current + HEADER_SIZE);
        }
        current = current->next;
    }

    return NULL;
}

void kfree(void* ptr) {
    if (ptr == NULL) return;

    BlockHeader* current = (BlockHeader*)((uint8_t*)ptr - HEADER_SIZE);
    current->is_free = true;

    busy_mem_size -= current->size;

    if (current->next != NULL && current->next->is_free) {
        current->size += HEADER_SIZE + current->next->size;
        current->next = current->next->next;
        if (current->next != NULL) {
            current->next->prev = current;
        }
    }

    if (current->prev != NULL && current->prev->is_free) {
        current->prev->size += HEADER_SIZE + current->size;
        current->prev->next = current->next;
        if (current->next != NULL) {
            current->next->prev = current->prev;
        }
    }
}

size_t get_busy_mem_size() {
    return busy_mem_size;
}
