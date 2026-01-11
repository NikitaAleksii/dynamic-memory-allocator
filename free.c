#include <stdio.h>
#include <stddef.h>
#include <unistd.h>

#include "./include/malloc.h"
#include "./include/avl.h"
#include "./include/free.h"

int in_heap(void *ptr)
{
    return ptr >= heap_lo() && ptr < heap_hi();
}

void *merge_blocks(void *block)
{
    void *prev = get_prev(block);
    void *next = get_next(block);

    int prev_free = in_heap(prev) && (get_alloc(prev) == 0);
    int next_free = in_heap(next) && (get_alloc(next) == 0);

    size_t size = get_size(get_hdrp(block));

    if (!prev_free && !next_free)
    {
        return block;
    }
    else if (prev_free && !next_free)
    {
        free_root = delete(free_root, (struct free_block *)prev);

        size += get_size(get_hdrp(prev));
        size_t packed = size & ~((size_t)1);

        write_size(get_hdrp(prev), packed);
        write_size(get_ftrp(block), packed);

        return prev;
    }
    else if (next_free && !prev_free)
    {
        free_root = delete(free_root, (struct free_block *)next);

        size += get_size(get_hdrp(next));
        size_t packed = size & ~((size_t)1);

        write_size(get_hdrp(block), packed);
        write_size(get_ftrp(next), packed);

        return block;
    }
    else
    {
        free_root = delete(free_root, (struct free_block *)prev);
        free_root = delete(free_root, (struct free_block *)next);

        size += get_size(get_hdrp(prev)) + get_size(get_hdrp(next));
        size_t packed = size & ~((size_t)1);

        write_size(get_hdrp(prev), packed);
        write_size(get_ftrp(next), packed);

        return prev;
    }
}

void my_free(void *data)
{
    if (data == NULL)
        return;

    size_t size = get_size(get_hdrp(data));
    size_t packed = size & ~((size_t)1);
    write_size(get_hdrp(data), packed);
    write_size(get_ftrp(data), packed);

    void *block = merge_blocks(data);

    struct free_block *node = (struct free_block *)block;
    node->size = get_size(get_hdrp(block));
    node->height = 1;
    node->left_block = NULL;
    node->right_block = NULL;

    free_root = insert(free_root, node);
    return;
}
