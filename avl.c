/*
 *  Implements an AVL (self-balancing BST) tree used
 *  to track FREE memory blocks for a custom malloc/free allocator.
 */

#include <stdio.h>
#include <stddef.h>
#include "./include/avl.h"

// Compares blocks by size; tie-breaks by address to ensure a strict total order.
// a > b returns 1      a < b return -1
int cmp(struct free_block *a, struct free_block *b)
{
    if (a->size < b->size)
        return -1;
    if (a->size > b->size)
        return 1;

    // tie-breaker by address
    if ((uintptr_t)a < (uintptr_t)b)
        return -1;
    if ((uintptr_t)a > (uintptr_t)b)
        return 1;
    return 0;
}

// Returns the AVL height of the node.
int height(struct free_block *block)
{
    if (block == NULL)
        return 0;
    return block->height;
}

// Returns balance factor = height(right) - height(left).
int getBalance(struct free_block *block)
{
    if (block == NULL)
        return 0;
    return height(block->right_block) - height(block->left_block);
}

// Gets maximum of two integers.
int max(int a, int b)
{
    return a > b ? a : b;
}

/*
 * Performs a left rotation on an AVL subtree.
 *
 * Parameters:
 *   root - Pointer to the root of the subtree to rotate.
 *
 * Returns:
 *   Pointer to the new root of the rotated subtree.
 */
struct free_block *rotate_left(struct free_block *root)
{
    if (!root || !root->right_block)
        return root;

    struct free_block *new_root = root->right_block;
    struct free_block *moved_subtree = new_root->left_block;

    // Perform rotation
    new_root->left_block = root;
    root->right_block = moved_subtree;

    // Update heights
    root->height = max(height(root->left_block), height(root->right_block)) + 1;
    new_root->height = max(height(new_root->left_block), height(new_root->right_block)) + 1;

    // Return new root
    return new_root;
}

/*
 * Performs a right rotation on an AVL subtree.
 *
 * Parameters:
 *   root - Pointer to the root of the subtree to rotate.
 *
 * Returns:
 *   Pointer to the new root of the rotated subtree.
 */
struct free_block *rotate_right(struct free_block *root)
{
    if (!root || !root->left_block)
        return root;

    struct free_block *new_root = root->left_block;
    struct free_block *moved_subtree = new_root->right_block;

    // Perform rotation
    new_root->right_block = root;
    root->left_block = moved_subtree;

    // Update heights
    root->height = max(height(root->left_block), height(root->right_block)) + 1;
    new_root->height = max(height(new_root->left_block), height(new_root->right_block)) + 1;

    // Return new root
    return new_root;
}

/*
 * Inserts `memoryBlock` into the AVL tree rooted at `root` and rebalances.
 *
 * Parameters:
 *   root        - Pointer to the root of a tree.
 *   memoryBlock - Pointer to the memory block to insert in a tree.
 *
 * Returns:
 *   Pointer to the root of the subtree after insertion.
 */
struct free_block *insert(struct free_block *root, struct free_block *memoryBlock)
{
    if (root == NULL)
    {
        root = memoryBlock;
        return root;
    }

    // Normal BST insertion
    if (cmp(memoryBlock, root) < 0)
    {
        root->left_block = insert(root->left_block, memoryBlock);
    }
    else
    {
        root->right_block = insert(root->right_block, memoryBlock);
    }

    // Update height
    root->height = max(height(root->left_block), height(root->right_block)) + 1;

    // Get balance of the memory block to check if it is unbalanced
    int balance = getBalance(root);

    // Left-Left
    if (balance < -1 && getBalance(root->left_block) < 0)
    {
        return rotate_right(root);
    }
    // Right-Right
    else if (balance > 1 && getBalance(root->right_block) > 0)
    {
        return rotate_left(root);
    }
    // Left-Right
    else if (balance < -1 && getBalance(root->left_block) > 0)
    {
        root->left_block = rotate_left(root->left_block);
        return rotate_right(root);
    }
    // Right-Left
    else if (balance > 1 && getBalance(root->right_block) < 0)
    {
        root->right_block = rotate_right(root->right_block);
        return rotate_left(root);
    }

    return root;
}

// Returns the free block with the smallest size in the subtree rooted at `root`.
struct free_block *minValue(struct free_block *root)
{
    struct free_block *current = root;

    while (current->left_block != NULL)
    {
        current = current->left_block;
    }

    return current;
}

/*
 * Removes the free memory block pointed to by `memoryBlock` from the AVL tree
 * rooted at `root`, rebalancing the tree.
 *
 * Parameters:
 *   root        - Pointer to the root of a tree.
 *   memoryBlock - Pointer to the memory block to remove.
 *
 * Returns:
 *   Pointer to the root of the subtree after removal.
 */
struct free_block *delete(struct free_block *root, struct free_block *memoryBlock)
{
    if (root == NULL)
        return root;

    // Compare a memory block to delete with a root
    int cmpBlocks = cmp(memoryBlock, root);

    // Normal BST Remove
    if (cmpBlocks < 0)
    {
        root->left_block = delete(root->left_block, memoryBlock);
    }
    else if (cmpBlocks > 0)
    {
        root->right_block = delete(root->right_block, memoryBlock);
    }
    else
    {
        // 0 or 1 child
        if (root->left_block == NULL || root->right_block == NULL)
        {
            struct free_block *temp = root->left_block ? root->left_block : root->right_block;

            if (temp == NULL)
            {
                temp = root;
                root = NULL;
            }
            else
            {
                root = temp;
            }
        }

        // 2 children
        else
        {
            struct free_block *temp = minValue(root->right_block);

            // Detach a successor
            root->right_block = delete(root->right_block, temp);

            // Reattach
            temp->left_block = root->left_block;
            temp->right_block = root->right_block;

            root = temp;
        }
    }

    // Check if root exists
    if (!root)
        return NULL;

    // Update height
    root->height = max(height(root->left_block), height(root->right_block)) + 1;

    // Get balance of the memory block to check if it is unbalanced
    int balance = getBalance(root);

    // Left-Left
    if (balance < -1 && getBalance(root->left_block) <= 0)
    {
        return rotate_right(root);
    }
    // Right-Right
    else if (balance > 1 && getBalance(root->right_block) >= 0)
    {
        return rotate_left(root);
    }
    // Left-Right
    else if (balance < -1 && getBalance(root->left_block) > 0)
    {
        root->left_block = rotate_left(root->left_block);
        return rotate_right(root);
    }
    // Right-Left
    else if (balance > 1 && getBalance(root->right_block) < 0)
    {
        root->right_block = rotate_right(root->right_block);
        return rotate_left(root);
    }

    return root;
}

/*
 * Finds the best-fit free block in the AVL tree: the smallest block whose size
 * is >= `size`.
 *
 * Parameters:
 *   root - Pointer to the root of a tree.
 *   size - Requested block size.
 *
 * Returns:
 *   Pointer to the best-fit free block, or NULL if no free block is large enough.
 */

struct free_block *best_fit(struct free_block *root, size_t size)
{
    struct free_block *best = NULL;

    while (root)
    {
        if (root->size >= size)
        {
            best = root;
            root = root->left_block;
        }
        else
        {
            root = root->right_block;
        }
    }
    return best;
}

// Removes best-fit memory block.
struct free_block *pop_best_fit(struct free_block **root, size_t size)
{
    struct free_block *b = best_fit(*root, size);
    if (b)
        *root = delete(*root, b);
    return b;
}