#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include "alloc.h"

#define MAX(a,b) (a>b?a:b)
#define MIN(a,b) (a<b?a:b)

#define ALLOC_ALLOCRE_COPY_THRESHOLD (4096)

#define ALLOC_NULLCHECK(ptr) {                       \
    typeof(ptr) p = ptr;                             \
    if (p == (void *) -1 || p == NULL) {             \
        fprintf(stderr, "liballoc: null pointer\n"); \
        abort();                                     \
    }                                                \
}

typedef struct ALLOC_mhead_st ALLOC_mhead_t;
typedef struct ALLOC_membloc_st ALLOC_membloc_t;

/* functions */
void ALLOC_clean();
ALLOC_membloc_t *ALLOC_mblock_find(void *ptr);
ALLOC_membloc_t *ALLOC_allocate_m(size_t size, void *ptr);
void ALLOC_linkup(ALLOC_membloc_t *node);

/** head of linked list */
struct ALLOC_mhead_st
{
    size_t blockc;
    ALLOC_membloc_t *start;
    ALLOC_membloc_t *end;
};

/** data of a memory block */
struct ALLOC_membloc_st
{
    bool free;
    void *ptr;
    size_t size;
    ALLOC_membloc_t *prv;
    ALLOC_membloc_t *nxt;
};

/** linked list of block data */
ALLOC_mhead_t *ALLOC_memhead = NULL;

/** searches for a specific block data based on its address */
ALLOC_membloc_t *ALLOC_mblock_find(void *ptr)
{
    typedef ALLOC_membloc_t *node_t;
    node_t p = ALLOC_memhead->start;
    while (p) {
        if (p->ptr == ptr) return p;
        p = p->nxt;
    }
    return NULL;
}

void ALLOC_allocate_head()
{
    if (!ALLOC_memhead) {
        ALLOC_memhead = sbrk(sizeof(ALLOC_mhead_t));
        ALLOC_NULLCHECK(ALLOC_memhead);
        ALLOC_memhead->blockc = 0;
        ALLOC_memhead->start = NULL;
        ALLOC_memhead->end = NULL;
    }
}

ALLOC_membloc_t *ALLOC_allocate_m(size_t size, void *ptr)
{
    ALLOC_NULLCHECK(ptr);
    ALLOC_membloc_t *node = sbrk(sizeof(ALLOC_membloc_t));
    ALLOC_NULLCHECK(node);
    node->ptr = ptr;
    node->size = size;
    node->free = false;
    return node;
}

void ALLOC_linkup(ALLOC_membloc_t *node)
{
    // setting last node links
    if (ALLOC_memhead->end)
        ALLOC_memhead->end->nxt = node;
    // setting new node links
    node->prv = ALLOC_memhead->end;
    node->nxt = NULL;
    // updating meta data at head
    if (!ALLOC_memhead->start)
        ALLOC_memhead->start = node;
    ALLOC_memhead->end = node;
    ALLOC_memhead->blockc++;
}

/** alloctes specified size */
void *allocm(size_t size)
{
    if (!ALLOC_memhead)
        ALLOC_allocate_head();

    typedef ALLOC_membloc_t *node_t;

    // attempting to recycle old empty block
    if (ALLOC_memhead->start) {
        node_t reusable = ALLOC_memhead->start;
        while (reusable)
            if (reusable->size >= size && reusable->free) break;
            else reusable = reusable->nxt;
        if (reusable) {
            reusable->free = false;
            if (reusable->size == size) return reusable->ptr;
            node_t leftover = (node_t) (reusable->ptr + size);
            leftover->free = true;
            leftover->ptr = (void *) (leftover + sizeof(ALLOC_membloc_t));
            leftover->size = reusable->size - size;
            leftover->prv = reusable;
            leftover->nxt = reusable->nxt;
            reusable->nxt = leftover;
            return reusable->ptr;
        }
    }

    // allocating new block
    void *ptr = sbrk(size);
    ALLOC_linkup(ALLOC_allocate_m(size, ptr));
    return ptr;
}

/** resizes allocated block if possible, or copies data around */
void *allocre(void *ptr, size_t size)
{
    if (!ptr) allocm(size);
    ALLOC_membloc_t *block = ALLOC_mblock_find(ptr);
    ALLOC_NULLCHECK(block);
    if (block->size > ALLOC_ALLOCRE_COPY_THRESHOLD && !block->nxt) {
        sbrk(size - block->size);
        block->size = size;
        return ptr;
    } else {
        void *newptr = allocm(size);
        memcpy(newptr, block->ptr, MIN(block->size, size));
        block->free = true;
        return newptr;
    }
    return NULL;
}

/** marks pointer to block for cleanup */
void alloc_free(void *ptr)
{
    if (!ptr) return;
    ALLOC_NULLCHECK(ptr);
    ALLOC_membloc_t *block = ALLOC_mblock_find(ptr);
    ALLOC_NULLCHECK(block);
    block->free = true;
    while (block && !block->nxt && block->free == true) {
        ALLOC_membloc_t *tofree = block;
        block = block->prv;
        if (tofree->prv) tofree->prv->nxt = NULL;
        else {
            ALLOC_memhead->start = NULL;
            ALLOC_memhead->end = NULL;
        }
        ALLOC_memhead->blockc--;
        brk(tofree);
    }
}
