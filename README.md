# Table of Contents
[AVL Tree](#avl-tree) \
[Malloc Implementation](#malloc) \
[Free Implementation](#free) \
[Realloc Implementation](#realloc) \
[Calloc Implementation](#calloc)

# Introduction
Dynamic memory allocation is the process of managing memory at runtime rather than at compile time.
It allows programs to allocate memory on the heap when the exact amount of required storage is not
known in advance. This approach provides flexibility and improves memory utilization in applications
with dynamic or unpredictable memory requirements.

In this project, I implemented a custom dynamic memory allocator that provides functionality similar
to standard library routines, including `malloc`, `free`, `realloc`, and `calloc`. The allocator
manages heap memory manually and demonstrates fundamental techniques used in real-world memory
management systems.

## Clone the Repository
```bash
git clone <https://github.com/NikitaAleksii/dynamic-memory-allocator.git>
```

## Compile Source Files
```bash
clang -I./include avl.c malloc.c free.c realloc.c calloc.c main.c -o allocator -Wno-deprecated-declarations
```
On macOS, sbrk is deprecated, so the **-Wno-deprecated-declarations flag** suppresses warnings.

<a id="avl-tree"></a>
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

### Interface

- `struct free_block *insert(struct free_block *root, struct free_block *memoryBlock);`  
  Inserts `memoryBlock` into the AVL tree and returns the new root.

- `struct free_block *delete(struct free_block *root, struct free_block *memoryBlock);`  
  Removes the exact node `memoryBlock` from the tree and returns the new root.

- `struct free_block *best_fit(struct free_block *root, size_t size);`  
  Returns the best-fit node: the smallest block with `root->size >= size`, or `NULL`.

- `struct free_block *pop_best_fit(struct free_block **root, size_t size);`  
  Finds and removes a best-fit block, returning the updated root.

<a id="malloc"></a>
## `malloc.c` — Custom Malloc Implementation

`malloc.c` implements a custom dynamic memory allocator (`my_malloc`).
It allocates memory using a **best-fit** policy backed by an **AVL tree** of free blocks.
When no suitable free block exists, the heap is extended using `sbrk()`.

### What this file is responsible for

- Heap memory allocation
- Block metadata management (headers and footers)
- Integration with the AVL free-block tree

### Design choices

**Block format**

- Header and footer store the **total block size**
- Lowest bit indicates allocation status (`1 = allocated`, `0 = free`)

**Alignment**

- All blocks are **16-byte aligned**
- Low bits are used safely for flags

**Minimum block size**

- Free blocks must hold a `struct free_block` in the payload
- Blocks smaller than `MIN_BLOCK_SIZE` are never split

**Allocation**

- Selects the smallest free block that satisfies the request (best-fit)
- Splits blocks when the remainder is large enough
- Extends the heap with `sbrk()` if no free block fits

### Interface

- `void *my_malloc(size_t size);`

<a id="free"></a>
## `free.c` — Custom Dynamic Memory Deallocation

`free.c` implements memory deallocation for blocks previously allocated by the custom dynamic memory allocator.
Free memory blocks are tracked in an **AVL tree**, enabling efficient lookup and insertion using a **best-fit** strategy.

### What this file is responsible for

- Releasing allocated memory blocks
- Coalescing (merging) adjacent free blocks to reduce fragmentation
- Reinserting merged blocks into the free-block AVL tree

### Design Choices

**Block Coalescing**

- Determines whether the previous and/or next memory blocks reside within heap bounds
- Merges blocks according to adjacency:
  - No merge
  - Merge with previous block
  - Merge with next block
  - Merge with both previous and next blocks

### Interface

- `void my_free(void *data);`

<a id="realloc"></a>
## `realloc.c` — Custom Dynamic Memory Reallocation

`realloc.c` implements memory reallocation for blocks previously allocated by the custom dynamic memory allocator.
It adjusts the size of allocated memory blocks to match a new requested payload size.

### What this file is responsible for

- Resizing allocated memory blocks
- Allocating a new block when **ptr** is **NULL**
- Freeing a block and returning **NULL** when the new size is **0**
- Preserving existing data when resizing memory blocks

### Design Choices

**Reallocation Strategy**

- If the requested size fits within the existing block, the block is kept in place
- When shrinking a block, the allocator splits the block if the remaining space is large enough to form a valid free block
- When growing a block, a new block is allocated, the old data is copied, and the original block is freed

### Interface

- `void *my_realloc(void *ptr, size_t new_payload);`

<a id="calloc"></a>
## `calloc.c` — Custom Zero-Initialized Memory Allocation

`calloc.c` implements zero-initialized memory allocation using the custom dynamic
memory allocator. It allocates a single contiguous memory block and initializes
all bytes to zero.

### What this file is responsible for

- Allocating memory for an array of elements
- Initializing allocated memory to zero
- Detecting and preventing integer overflow during size computation

### Interface

- `void *my_calloc(size_t nitems, size_t size);`
