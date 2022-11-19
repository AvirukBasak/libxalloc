#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include "alloc.h"

#define MAX(a,b) (a>b?a:b)
#define MIN(a,b) (a<b?a:b)

#define ALLOC_ALLOCRE_COPY_THRESHOLD (4096)
#define ALLOC_BLOCKHEAD_PADDING (16)

#define ALLOC_NULLCHECK(ptr) {                    \
    typeof(ptr) _p = ptr;                         \
    if (_p == (void*) -1 || _p == NULL) {         \
        write(2, "liballoc: null pointer\n", 23); \
        abort();                                  \
    }                                             \
}

typedef struct ALLOC_mhead_st ALLOC_mhead_t;
typedef struct ALLOC_membloc_st ALLOC_membloc_t;
typedef ALLOC_membloc_t *node_t;

/** linked list of block data */
ALLOC_mhead_t *ALLOC_memhead = NULL;

/* functions */
void ALLOC_allocate_head();
pointer_t ALLOC_allocate_new(size_t size);
void ALLOC_linkup(ALLOC_membloc_t *node);
ALLOC_membloc_t *ALLOC_mblock_find(pointer_t ptr);
ALLOC_membloc_t *ALLOC_mblock_split(ALLOC_membloc_t *block, size_t required_sz);
ALLOC_membloc_t *ALLOC_mblock_merge(ALLOC_membloc_t *block, size_t required_sz);

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
    char padding[ALLOC_BLOCKHEAD_PADDING];
    bool free;
    pointer_t ptr;
    size_t size;
    ALLOC_membloc_t *prv;
    ALLOC_membloc_t *nxt;
};

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

pointer_t ALLOC_allocate_new(size_t size)
{
    ALLOC_membloc_t *node = sbrk(sizeof(ALLOC_membloc_t));
    ALLOC_NULLCHECK(node);
    pointer_t ptr = sbrk(size);
    ALLOC_NULLCHECK(ptr);
    node->ptr = ptr;
    node->size = size;
    node->free = false;
    ALLOC_linkup(node);
    return ptr;
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

/** searches for a specific block data based on its address */
ALLOC_membloc_t *ALLOC_mblock_find(pointer_t ptr)
{
    node_t p = ALLOC_memhead->start;
    while (p) {
        if (p->ptr == ptr) return p;
        p = p->nxt;
    }
    return NULL;
}

ALLOC_membloc_t *ALLOC_mblock_split(ALLOC_membloc_t *block, size_t required_sz)
{
    ALLOC_NULLCHECK(block);
    if (required_sz >= block->size) return NULL;
    size_t leftover_sz = block->size - required_sz - sizeof(ALLOC_membloc_t);
    /* if remaining memory is less than double the size of a memory head,
     * then no changes are made
     */
    if (leftover_sz < 2 * sizeof(ALLOC_membloc_t)) return block;
    node_t leftover = (node_t) (block->ptr + required_sz);
    leftover->free = true;
    leftover->ptr = (pointer_t) (leftover + sizeof(ALLOC_membloc_t));
    leftover->size = leftover_sz;
    leftover->prv = block;
    leftover->nxt = block->nxt;
    block->nxt = leftover;
    block->size = required_sz;
    return block;
}

ALLOC_membloc_t *ALLOC_mblock_merge(ALLOC_membloc_t *block, size_t required_sz)
{
    ALLOC_NULLCHECK(block);
    if (required_sz <= block->size) return block;
    size_t available_sz = block->size;
    node_t node = block->nxt;
    while (available_sz < required_sz && node && node->free) {
        available_sz += node->size;
        node = node->nxt;
    }
    if (available_sz < required_sz)
        return NULL;
    if (available_sz > required_sz) {
        node = ALLOC_mblock_split(node, node->size - (available_sz - required_sz));
        node->nxt->prv = block;
    }
    block->nxt = node->nxt;
    block->size = required_sz;
    return block;
}

/** alloctes specified size */
pointer_t allocm(size_t size)
{
    if (!ALLOC_memhead)
        ALLOC_allocate_head();

    // attempting to recycle old empty block
    if (ALLOC_memhead->start) {
        node_t reusable = ALLOC_memhead->start;
        while (reusable)
            if (reusable->size >= size && reusable->free) break;
            else reusable = reusable->nxt;
        if (reusable) {
            reusable->free = false;
            if (reusable->size == size) return reusable->ptr;
            return ALLOC_mblock_split(reusable, size)->ptr;
        }
    }

    // fallback: allocating new block
    return ALLOC_allocate_new(size);
}

/** resizes allocated block if possible, or copies data around */
pointer_t allocre(pointer_t ptr, size_t size)
{
    if (!ptr) allocm(size);
    ALLOC_membloc_t *block = ALLOC_mblock_find(ptr);
    ALLOC_NULLCHECK(block);
    if (block->size == size) return ptr;

    // splitting blocks if new size is smaller
    if (size < block->size)
        return ALLOC_mblock_split(block, size)->ptr;

    // if block is too large
    if (block->size > ALLOC_ALLOCRE_COPY_THRESHOLD) {
        // last block: update brk
        if (!block->nxt) {
            sbrk(size - block->size);
            block->size = size;
            return ptr;
        }
        // consecutive empty blocks
        else if (block->nxt->free) {
            node_t merged = ALLOC_mblock_merge(block, size);
            if (merged) return merged->ptr;
        }
    }

    // fallback: new block allocation and copy data
    pointer_t newptr = allocm(size);
    ALLOC_NULLCHECK(newptr);
    memcpy(newptr, block->ptr, MIN(block->size, size));
    block->free = true;
    return newptr;
}

/** marks pointer to block for cleanup */
void alloc_free(pointer_t ptr)
{
    if (!ptr) return;
    ALLOC_NULLCHECK(ptr);
    ALLOC_membloc_t *block = ALLOC_mblock_find(ptr);
    ALLOC_NULLCHECK(block);
    block->free = true;

    // cleaning up free blocks from the end of list
    block = ALLOC_memhead->end;
    while (block && !block->nxt && block->free == true) {
        ALLOC_membloc_t *tofree = block;
        ALLOC_memhead->end = block = tofree->prv;
        if (tofree->prv) tofree->prv->nxt = NULL;
        else {
            ALLOC_memhead->start = NULL;
            ALLOC_memhead->end = NULL;
        }
        ALLOC_memhead->blockc--;
        brk(tofree);
    }
}
