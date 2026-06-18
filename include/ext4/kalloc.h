#ifndef _EXT4_KALLOC_H
#define _EXT4_KALLOC_H

#include "types.h"
#define KALLOC_SIZE 0x1000 // 4KB

typedef void *(*kalloc_alloc_fn)(uint64 size);
typedef void (*kalloc_free_fn)(void *ptr);

void kalloc_set_allocator(kalloc_alloc_fn alloc, kalloc_free_fn free);

void *kalloc();
void kfree(void *ptr);

#endif
