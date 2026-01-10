#include <stddef.h>

// Free Block of Memory
struct free_block
{
    size_t size;
    int height;

    struct free_block *left_block;
    struct free_block *right_block;
};

struct free_block *insert(struct free_block *root, struct free_block *memoryBlock);
struct free_block *delete(struct free_block *root, struct free_block *memoryBlock);
struct free_block *best_fit(struct free_block *root, size_t size);
struct free_block *pop_best_fit(struct free_block **root, size_t size);