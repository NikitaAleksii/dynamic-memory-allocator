#ifndef MALLOC_H
#define MALLOC_H

#include <stddef.h>
#include "./avl.h"

// Rounding up to the closest multiple of 16
#define ALIGN 16
#define ALIGN_UP(x) (((x) + (ALIGN - 1)) & ~(ALIGN - 1))

#define HEADER_SIZE sizeof(size_t)
#define FOOTER_SIZE sizeof(size_t)

// Minimum size of a payload is at least of the size of a free memory block
#define MIN_PAYLOAD_SIZE ALIGN_UP(sizeof(struct free_block))
#define MIN_BLOCK_SIZE ALIGN_UP(HEADER_SIZE + FOOTER_SIZE + MIN_PAYLOAD_SIZE)

// Mask to extract block size (lower ALIGN bits hold alloc bit flag.
#define SIZE_MASK (~(size_t)(ALIGN - 1))

// Returns value (not size because the first bit serves as the indicator of whether the block is allocated or free)
// of header/footer
size_t get_value(void *ptr)
{
    return *(size_t *)ptr;
}
// Writes size to header/footer
void write_size(void *ptr, size_t size)
{
    *(size_t *)ptr = size;
}
// Returns total block size stored in a header/footer word
size_t get_size(void *ptr)
{
    return get_value(ptr) & SIZE_MASK;
}

// Returns pointer to header
void *get_hdrp(void *data)
{
    return (char *)data - HEADER_SIZE;
}
// Returns pointer to footer
void *get_ftrp(void *data)
{
    return (char *)data + get_size(get_hdrp(data)) - FOOTER_SIZE - HEADER_SIZE;
}

int get_alloc(void *data)
{
    return get_value(get_hdrp(data)) & 1;
}

void *get_next(void *data)
{
    return (char *)data + get_size(get_hdrp(data));
}
void *get_prev(void *data)
{
    return (char *)data - get_size((char *)data - HEADER_SIZE - FOOTER_SIZE);
}

extern struct free_block *free_root;

void *heap_lo();
void *heap_hi();

void *my_malloc(size_t size);

#endif