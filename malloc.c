#include <stdio.h>
#include <stddef.h>
#include <unistd.h>

#include "./include/malloc.h"

// Root of an AVL tree of free memory blocks
struct free_block *free_root = NULL;

static void *g_heap_lo = NULL;
static void *g_heap_hi = NULL;

void *heap_lo() { return g_heap_lo; }
void *heap_hi() { return g_heap_hi; }

/*
 * Extends the heap using sbrk() and creates a single allocated block
 *
 * The header and footer store the total block size (including header + payload + footer)
 * and the low bit is set to 1 to mark the block as allocated
 *
 * Parameters:
 *   size - total block size in bytes (already aligned)
 *
 * Returns:
 *   Payload pointer to the newly allocated block, or NULL on failure
 */
void *allocate_heap(size_t size)
{
    // Allocate memory
    void *allocated_block = sbrk(size);

    if (allocated_block == (void *)-1)
    {
        perror("sbrk failed");
        return NULL;
    }

    if (!g_heap_lo)
        g_heap_lo = allocated_block;
    g_heap_hi = sbrk(0);

    allocated_block = (char *)allocated_block + HEADER_SIZE;

    // Write size to header and footer
    size_t packed = size | 1; // set alloc bit (bit 0) to mark allocated
    write_size(get_hdrp(allocated_block), packed);
    write_size(get_ftrp(allocated_block), packed);
    return allocated_block;
}

/*
 * Splits a free block into an allocated block of size `size` (total block size)
 * and a remainder free block, if the remainder is at least MIN_BLOCK_SIZE
 *
 * The allocated portion is returned to the caller. The remainder block is
 * initialized as a free_block node and inserted into the AVL free tree
 *
 * Parameters:
 *   free_block - pointer to the payload of a free block (already removed from the AVL tree)
 *   size       - total size (aligned) for the allocated block
 *
 * Returns:
 *   Payload pointer to the allocated block
 */
void *split_block(struct free_block *free_block, size_t size)
{
    void *block = free_block;
    size_t free_size = free_block->size;
    size_t rem = free_size - size;

    // Allocate needed amount of memory
    write_size(get_hdrp(block), size | 1);
    write_size(get_ftrp(block), size | 1);

    // Assign the remaining size to another block
    void *rem_block = (char *)block + size;
    write_size(get_hdrp(rem_block), rem | 0);
    write_size(get_ftrp(rem_block), rem | 0);

    // Add the remaining block to the AVL tree of free memory blocks
    struct free_block *new_free_block = (struct free_block *)rem_block;
    new_free_block->size = rem;
    new_free_block->height = 1;
    new_free_block->left_block = NULL;
    new_free_block->right_block = NULL;

    free_root = insert(free_root, new_free_block);

    return block;
}

/*
 * Allocates a block with at least `size` bytes of payload.
 *
 * The allocator rounds up the requested size to include header/footer
 * and alignment, then searches the AVL tree for a best-fit free block.
 * If found, the block may be split; otherwise the entire block is allocated.
 * If no suitable free block exists, the heap is extended via sbrk().
 *
 * Parameters:
 *   size - requested payload size in bytes
 *
 * Returns:
 *   Payload pointer to allocated memory, or NULL on failure.
 */
void *my_malloc(size_t size)
{
    // Compute total block size (header+footer+payload) and round up for alignment
    size_t block_size = ALIGN_UP(HEADER_SIZE + FOOTER_SIZE + size);

    // Check if the rounded size satisfies minimum block size
    if (block_size < MIN_BLOCK_SIZE)
    {
        block_size = MIN_BLOCK_SIZE;
    }

    // Find a free memory block in an AVL tree using best-fit algorithm
    struct free_block *best_block = pop_best_fit(&free_root, block_size);

    // If not found, access heap to allocate memory
    if (best_block == NULL)
    {
        return allocate_heap(block_size);
    }

    // Check if the size of the remaining block is enough
    if (best_block->size - block_size >= MIN_BLOCK_SIZE)
    {
        return split_block(best_block, block_size);
    }
    else
    {
        // If not allocate the whole free memory block
        void *block = best_block;
        write_size(get_hdrp(block), best_block->size | 1);
        write_size(get_ftrp(block), best_block->size | 1);
        return block;
    }
}
