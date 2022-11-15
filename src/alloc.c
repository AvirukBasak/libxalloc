#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include "libavl/avl.h"
#include "alloc.h"

#ifndef ALLOC_CLEANUP_TRIGGERC
#define ALLOC_CLEANUP_TRIGGERC (32)
#endif

#define MAX(a,b) (a>b?a:b)
#define MIN(a,b) (a<b?a:b)

typedef struct ALLOC_mhead_st ALLOC_mhead_t;
typedef struct ALLOC_membloc_st ALLOC_membloc_t;

/* functions */
void ALLOC_clean();
ALLOC_membloc_t *ALLOC_mblock_find(void *ptr);

/** head of linked list */
struct ALLOC_mhead_st
{
    ALLOC_membloc_t *start;
    ALLOC_membloc_t *end;
    size_t blockc;
};

/** data of a memory block */
struct ALLOC_membloc_st
{
    ALLOC_membloc_t *prv;
    void *ptr;
    size_t size;
    bool free;
    ALLOC_membloc_t *nxt;
};

/** linked list of block data */
ALLOC_mhead_t *ALLOC_memory = NULL;

/** cleanup trigger count; when count reaches 0, cleanup function is called; count is then reset
    by default, cleanup is triggered after a total of 32 alloc_* calls */
size_t ALLOC_triggerc = ALLOC_CLEANUP_TRIGGERC;

/** cleans up heap and if possible reduces heap break point */
void ALLOC_clean()
{
    ALLOC_triggerc = ALLOC_CLEANUP_TRIGGERC;
}

/** searches for a specific block data based on its address */
ALLOC_membloc_t *ALLOC_mblock_find(void *ptr)
{
    typedef ALLOC_membloc_t *node;
    node p = ALLOC_memory->start;
    while (p) {
        if (p->ptr == ptr) return p;
        p = p->nxt;
    }
    return NULL;
}

/** alloctes specified size */
void *alloc_m(size_t size)
{
    if (!ALLOC_memory) {
        ALLOC_memory = sbrk(sizeof(ALLOC_mhead_t));
        ALLOC_memory->blockc = 0;
        ALLOC_memory->start = NULL;
        ALLOC_memory->end = NULL;
    }
    ALLOC_membloc_t *allocator = sbrk(sizeof(ALLOC_membloc_t));
    if (allocator == (void *) -1) abort();
    void *ptr = sbrk(size);
    if (ptr == (void *) -1) abort();
    allocator->ptr = ptr;
    allocator->size = size;
    allocator->free = false;
    allocator->nxt = ALLOC_memory->start;
    allocator->prv = NULL;
    ALLOC_memory->start = allocator;
    ALLOC_memory->blockc++;
    return ptr;
}

/** resizes allocated block if possible, or copies data around */
void *alloc_re(void *ptr, size_t size)
{
    return NULL;
}

/** marks pointer to block for cleanup */
void alloc_free(void *ptr)
{
    if (!ptr) return;
    if (ptr == (void *) -1) abort();
    ALLOC_membloc_t *block = ALLOC_mblock_find(ptr);
    block->free = true;
    ALLOC_triggerc--;
    if (!ALLOC_triggerc) ALLOC_clean();
}

/** explicitly runs cleanup */
void alloc_cleanf()
{
    ALLOC_clean();
}
