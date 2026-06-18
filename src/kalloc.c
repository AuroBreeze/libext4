#include "kalloc.h"

static kalloc_alloc_fn current_alloc = 0;
static kalloc_free_fn current_free = 0;

void kalloc_set_allocator(kalloc_alloc_fn alloc, kalloc_free_fn free)
{
	current_alloc = alloc ? alloc : 0;
	current_free = free ? free : 0;
}

void *kalloc() { return current_alloc ? current_alloc(KALLOC_SIZE) : 0; }

void kfree(void *ptr)
{
	if (current_free) {
		current_free(ptr);
	}
}
