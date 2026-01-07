# custom-malloc
A custom dynamic memory allocator in C using an AVL tree for best-fit allocation.

## `avl.c` — Free-Block Memory AVL Tree

`avl.c` implements an AVL (self-balancing BST) used by the allocator to manage **free memory blocks** efficiently.  
The allocator uses this tree to find a **best-fit** block (the smallest free block that can satisfy a request) in `O(log n)` time.

### What this file is responsible for
- Maintaining an AVL tree of `struct free_block` nodes
- Supporting:
  - insertion of a freed/coalesced block
  - deletion of an allocated block
  - best-fit search 

### Design choices
- **Stateless API:** `avl.c` does not store global state.  
  The allocator owns the root pointer and passes it into AVL functions.  
  This makes the AVL code easier to test and reuse.
- **Ordering:** nodes are ordered primarily by `size`.  
  If multiple blocks have the same size, a tie-breaker (the block address) is used to ensure a strict ordering.
- **Balance factor convention:**  
  `balance(node) = height(node->right) - height(node->left)`

### Functions
- `struct free_block *insert(struct free_block *root, struct free_block *memoryBlock);`  
  Inserts `memoryBlock` into the AVL tree and returns the new root.

- `struct free_block *delete(struct free_block *root, struct free_block *memoryBlock);`  
  Removes the exact node `memoryBlock` from the tree and returns the new root.

- `struct free_block *best_fit(struct free_block *root, size_t size);`  
  Returns the best-fit node: the smallest block with `root->size >= size`, or `NULL`.

- `struct free_block *pop_best_fit(struct free_block *root, size_t size);`  
  Finds and removes a best-fit block, returning the updated root.
