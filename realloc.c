#include <stddef.h>
#include <string.h>

#include "./include/malloc.h"
#include "./include/free.h"

/*
 * Resizes an allocated block.
 *
 * - If ptr is NULL, behaves like my_malloc(new_payload).
 * - If new_payload is 0, frees ptr and returns NULL.
 * - If the requested size fits in the current block, the block is kept in place.
 *   If the leftover space is large enough to form a valid free block, the block is
 *   split and the remainder is freed.
 * - If the requested size does not fit, allocates a new block, copies the old payload,
 *   frees the old block, and returns the new block.
 *
 * Parameters:
 *   ptr          - Pointer to an allocated payload returned by my_malloc.
 *   new_payload  - New requested payload size.
 *
 * Returns:
 *   Pointer to the resized block's payload, or NULL on failure / when new_payload == 0.
 */
void *my_realloc(void *ptr, size_t new_payload)
{
    if (ptr == NULL)
    {
        return my_malloc(new_payload);
    }

    if (new_payload == 0)
    {
        my_free(ptr);
        return NULL;
    }

    // Old memory block size and payload size
    size_t old_block_size = get_size(get_hdrp(ptr));
    size_t old_payload = old_block_size - HEADER_SIZE - FOOTER_SIZE;

    // New memory block size
    size_t new_block_size = ALIGN_UP(new_payload + HEADER_SIZE + FOOTER_SIZE);
    if (new_block_size < MIN_BLOCK_SIZE)
        new_block_size = MIN_BLOCK_SIZE;

    if (new_block_size <= old_block_size)
    {
        // Check if there is enough space for a remaining block
        size_t remainder_size = old_block_size - new_block_size;

        // If not, return the old memory block
        if (remainder_size < MIN_BLOCK_SIZE)
        {
            return ptr;
        }
        // If yes, split the old memory block in two parts
        else
        {
            write_size(get_hdrp(ptr), new_block_size | 1);
            write_size(get_ftrp(ptr), new_block_size | 1);

            // Find the remaining block
            void *remaining_block = get_next(ptr);

            write_size(get_hdrp(remaining_block), remainder_size & ~((size_t)1));
            write_size(get_ftrp(remaining_block), remainder_size & ~((size_t)1));

            my_free(remaining_block);

            return ptr;
        }
    }
    // If there is not enough space in the old memory block for a new memory block
    else
    {
        void *new_block = my_malloc(new_payload);

        if (new_block == NULL)
            return NULL;

        // Copy only as much as fits in the new payload request
        size_t copy_sz = old_payload;
        if (new_payload < copy_sz)
            copy_sz = new_payload;
        memcpy(new_block, ptr, copy_sz);

        my_free(ptr);
        return new_block;
    }
}