#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include "alloc.h"

#define MAX(a,b) (a>b?a:b)
#define MIN(a,b) (a<b?a:b)

#define ALLOC_NULLCHECK(ptr) {              \
    typeof(ptr) p = ptr;                    \
    if (p == (void *) -1 || p == NULL) {    \
        fprintf(stderr, "null pointer\n");  \
        abort();                            \
    }                                       \
}

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
        ALLOC_NULLCHECK(ALLOC_memory);
        ALLOC_memory->blockc = 0;
        ALLOC_memory->start = NULL;
        ALLOC_memory->end = NULL;
    }
    ALLOC_membloc_t *allocator = sbrk(sizeof(ALLOC_membloc_t));
    ALLOC_NULLCHECK(allocator);
    void *ptr = sbrk(size);
    ALLOC_NULLCHECK(ptr);
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
    ALLOC_NULLCHECK(ptr);
    ALLOC_membloc_t *block = ALLOC_mblock_find(ptr);
    block->free = true;
}
