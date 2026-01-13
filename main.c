#include "./include/malloc.h"
#include "./include/free.h"
#include "./include/realloc.h"
#include "./include/calloc.h"

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

static void print_block_tags(const char *command, void *ptr) {
    assert(ptr != NULL);
    size_t hs = get_size(get_hdrp(ptr));
    size_t fs = get_size(get_ftrp(ptr));
    size_t hv = get_value(get_hdrp(ptr));
    size_t fv = get_value(get_ftrp(ptr));
    printf("%s p=%p | hdr(size=%zu,val=%zu) ftr(size=%zu,val=%zu)\n",
           command, ptr, hs, hv, fs, fv);
}

// For allocated blocks, header and footer should match exactly 
static void assert_tags_match(void *ptr) {
    assert(ptr != NULL);
    size_t hv = get_value(get_hdrp(ptr));
    size_t fv = get_value(get_ftrp(ptr));
    if (hv != fv) {
        print_block_tags("Value Mismatch!", ptr);
    }
    assert(hv == fv);
}

// Check alloc bit 
static void assert_allocated(void *ptr) {
    assert(ptr != NULL);
    assert((get_value(get_hdrp(ptr)) & 1) == 1);
}

static void fill_pattern(void *p, size_t n, uint8_t byte) {
    assert(p != NULL);
    memset(p, byte, n);
}

static void assert_pattern(void *p, size_t n, uint8_t byte) {
    assert(p != NULL);
    const uint8_t *b = (const uint8_t *)p;
    for (size_t i = 0; i < n; i++) {
        if (b[i] != byte) {
            printf("Pattern mismatch at i=%zu: got=%u expected=%u\n",
                   i, (unsigned)b[i], (unsigned)byte);
            assert(0);
        }
    }
}

/* ---------- Tests ---------- */

static void test_malloc_free_basic(void) {
    printf("\n=== test_malloc_free_basic ===\n");

    void *p = my_malloc(32);
    assert(p != NULL);
    print_block_tags("malloc(32)", p);
    assert_allocated(p);
    assert_tags_match(p);

    fill_pattern(p, 32, 0xAB);
    assert_pattern(p, 32, 0xAB);

    my_free(p);

    printf("free OK\n");
}

static void test_calloc_zeroing(void) {
    printf("\n=== test_calloc_zeroing ===\n");

    size_t nitems = 5, sz = 32;
    void *p = my_calloc(nitems, sz);
    assert(p != NULL);
    print_block_tags("calloc(5,32)", p);
    assert_allocated(p);
    assert_tags_match(p);

    // Must be all zeros
    uint8_t *b = (uint8_t *)p;
    for (size_t i = 0; i < nitems * sz; i++) {
        if (b[i] != 0) {
            printf("calloc not zero at i=%zu got=%u\n", i, (unsigned)b[i]);
            assert(0);
        }
    }

    my_free(p);
    printf("calloc zeroing OK\n");
}

static void test_realloc_grow_preserves_data(void) {
    printf("\n=== test_realloc_grow_preserves_data ===\n");

    void *p = my_malloc(64);
    assert(p != NULL);
    fill_pattern(p, 64, 0x5A);

    void *q = my_realloc(p, 2000); // force grow
    assert(q != NULL);
    print_block_tags("realloc(64->2000)", q);
    assert_allocated(q);
    assert_tags_match(q);

    // Old 64 bytes must remain
    assert_pattern(q, 64, 0x5A);

    my_free(q);
    printf("realloc grow preserve OK\n");
}

static void test_realloc_split_and_coalesce(void) {
    printf("\n=== test_realloc_split_and_coalesce ===\n");

    // Allocate two blocks back-to-back for adjacency effects
    void *a = my_malloc(256);
    void *b = my_malloc(256);
    assert(a && b);
    fill_pattern(a, 256, 0x11);
    fill_pattern(b, 256, 0x22);

    void *a2 = my_realloc(a, 64);
    assert(a2 != NULL);
    assert_pattern(a2, 64, 0x11);
    assert_tags_match(a2);

    // Free b and then free a2; this should coalesce a2 + remainder + b 
    my_free(b);
    my_free(a2);

    // Now allocate something about the combined size
    void *big = my_malloc(400);
    assert(big != NULL);
    print_block_tags("malloc(400) after coalesce", big);
    assert_tags_match(big);

    my_free(big);
    printf("realloc split + coalesce OK\n");
}

int main(void) {
    printf("Allocator tests starting...\n");

    test_malloc_free_basic();
    test_calloc_zeroing();
    test_realloc_grow_preserves_data();
    test_realloc_split_and_coalesce();

    printf("\nAll tests passed.\n");
    return 0;
}
