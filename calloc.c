#include <string.h>
#include <stdint.h>

#include "./include/malloc.h"
#include "./include/free.h"

/*
 * Allocates a zero-initialized block of memory for an array of `nitems` elements,
 * each of size `size` bytes.
 *
 * Parameters:
 *   nitems - Number of elements to allocate
 *   size   - Size in bytes of each element
 *
 * Returns:
 *   Pointer to a allocated block of memory with all bytes set to 0,
 *   or NULL if the allocation fails, an overflow occurs, or either
 *   nitems or size is zero.
 */
void *my_calloc(size_t nitems, size_t size)
{
    if (nitems == 0 || size == 0)
        return NULL;

    // Check if it's possible to allocate nitems with size `size`
    if (nitems > SIZE_MAX / size)
        return NULL;

    size_t total_size = nitems * size;

    void *block = my_malloc(total_size);
    if (!block)
        return NULL;

    // Set memory value to 0
    memset(block, 0, total_size);

    return block;
}