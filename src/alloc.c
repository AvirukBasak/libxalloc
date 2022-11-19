#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "alloc.h"

#define MAX(a,b) (a>b?a:b)
#define MIN(a,b) (a<b?a:b)

#define ALLOC_COPY_THRESHOLD (4096)
#define ALLOC_MBLOCK_PADDING (16)

#define ALLOC_NULLCHECK(ptr) if (!ptr || ptr == (void*) -1) _ALLOC_abortln("null pointer")

typedef struct ALLOC_mhead_t ALLOC_mhead_t;
typedef struct ALLOC_mblock_t ALLOC_mblock_t;
typedef ALLOC_mblock_t *node_t;

/** linked list of block data */
ALLOC_mhead_t *ALLOC_mhead = NULL;

/* functions */
void _ALLOC_abortln(const char *s);
void _ALLOC_mhead_allocate();
pointer_t _ALLOC_mblock_new(size_t size);
void _ALLOC_mblock_link(ALLOC_mblock_t *node);
ALLOC_mblock_t *_ALLOC_mblock_find(pointer_t ptr);
ALLOC_mblock_t *_ALLOC_mblock_split(ALLOC_mblock_t *block, size_t required_sz);
ALLOC_mblock_t *_ALLOC_mblock_merge(ALLOC_mblock_t *block, size_t required_sz);

/** head of linked list */
struct ALLOC_mhead_t
{
    size_t blockc;
    ALLOC_mblock_t *start;
    ALLOC_mblock_t *end;
};

/** data of a memory block */
struct ALLOC_mblock_t
{
    char padding[ALLOC_MBLOCK_PADDING];
    bool isfree;
    pointer_t ptr;
    size_t size;
    ALLOC_mblock_t *prv;
    ALLOC_mblock_t *nxt;
};

/** Abort with error message */
void _ALLOC_abortln(const char *s)
{
    size_t len = strlen(s);
    write(2, "liballoc: aborted: ", 19);
    write(2, s, len);
    write(2, "\n", 1);
    abort();
}

void _ALLOC_mhead_allocate()
{
    if (ALLOC_mhead) return;
    ALLOC_mhead = sbrk(sizeof(ALLOC_mhead_t));
    ALLOC_NULLCHECK(ALLOC_mhead);
    ALLOC_mhead->blockc = 0;
    ALLOC_mhead->start = NULL;
    ALLOC_mhead->end = NULL;
}

pointer_t _ALLOC_mblock_new(size_t size)
{
    ALLOC_mblock_t *node = sbrk(sizeof(ALLOC_mblock_t));
    ALLOC_NULLCHECK(node);
    pointer_t ptr = sbrk(size);
    ALLOC_NULLCHECK(ptr);
    node->ptr = ptr;
    node->size = size;
    node->isfree = false;
    _ALLOC_mblock_link(node);
    return ptr;
}

void _ALLOC_mblock_link(ALLOC_mblock_t *node)
{
    // setting last node links
    if (ALLOC_mhead->end)
        ALLOC_mhead->end->nxt = node;
    // setting new node links
    node->prv = ALLOC_mhead->end;
    node->nxt = NULL;
    // updating meta data at head
    if (!ALLOC_mhead->start)
        ALLOC_mhead->start = node;
    ALLOC_mhead->end = node;
    ALLOC_mhead->blockc++;
}

/** searches for a specific block data based on its address */
ALLOC_mblock_t *_ALLOC_mblock_find(pointer_t ptr)
{
    node_t p = ALLOC_mhead->start;
    while (p) {
        if (p->ptr == ptr) return p;
        p = p->nxt;
    }
    _ALLOC_abortln("invalid pointer");
    return NULL;
}

ALLOC_mblock_t *_ALLOC_mblock_split(ALLOC_mblock_t *block, size_t required_sz)
{
    ALLOC_NULLCHECK(block);
    if (required_sz == block->size) return block;
    if (required_sz > block->size) _ALLOC_abortln("size post split exceeds available size");
    size_t leftover_sz = block->size - required_sz - sizeof(ALLOC_mblock_t);
    /* if remaining memory is less-equal double the size of a memory head,
     * then no changes are made
     */
    if (leftover_sz <= 2 * sizeof(ALLOC_mblock_t)) return block;
    node_t leftover = (node_t) (block->ptr + required_sz);
    leftover->isfree = true;
    leftover->ptr = (pointer_t) (leftover + sizeof(ALLOC_mblock_t));
    leftover->size = leftover_sz;
    leftover->prv = block;
    leftover->nxt = block->nxt;
    block->nxt = leftover;
    block->size = required_sz;
    return block;
}

ALLOC_mblock_t *_ALLOC_mblock_merge(ALLOC_mblock_t *block, size_t required_sz)
{
    ALLOC_NULLCHECK(block);
    if (required_sz <= block->size) return block;
    size_t available_sz = block->size;
    node_t node = block->nxt;
    while (available_sz < required_sz && node && node->isfree) {
        available_sz += node->size;
        node = node->nxt;
    }
    if (available_sz < required_sz)
        return NULL;
    if (available_sz > required_sz) {
        node = _ALLOC_mblock_split(node, node->size - (available_sz - required_sz));
        node->nxt->prv = block;
    }
    block->nxt = node->nxt;
    block->size = required_sz;
    return block;
}

/** alloctes specified size */
pointer_t allocm(size_t size)
{
    if (!ALLOC_mhead)
        _ALLOC_mhead_allocate();

    // attempting to recycle old empty block
    if (ALLOC_mhead->start) {
        node_t reusable = ALLOC_mhead->start;
        while (reusable)
            if (reusable->isfree && reusable->size >= size) break;
            else reusable = reusable->nxt;
        if (reusable) {
            reusable->isfree = false;
            if (reusable->size == size) return reusable->ptr;
            return _ALLOC_mblock_split(reusable, size)->ptr;
        }
    }

    // fallback: allocating new block
    return _ALLOC_mblock_new(size);
}

/** resizes allocated block if possible, or copies data around */
pointer_t allocre(pointer_t ptr, size_t size)
{
    if (!ptr) allocm(size);
    ALLOC_mblock_t *block = _ALLOC_mblock_find(ptr);
    if (block->size == size) return ptr;

    // splitting blocks if new size is smaller
    if (size < block->size)
        return _ALLOC_mblock_split(block, size)->ptr;

    // if block is too large
    if (block->size > ALLOC_COPY_THRESHOLD) {
        // last block: update brk
        if (!block->nxt) {
            sbrk(size - block->size);
            block->size = size;
            return ptr;
        }
        // consecutive empty blocks
        else if (block->nxt->isfree) {
            node_t merged = _ALLOC_mblock_merge(block, size);
            if (merged) return merged->ptr;
        }
    }

    // fallback: new block allocation and copy data
    pointer_t newptr = allocm(size);
    memcpy(newptr, block->ptr, MIN(block->size, size));
    block->isfree = true;
    return newptr;
}

/** marks pointer to block for cleanup */
void alloc_free(pointer_t ptr)
{
    if (!ptr) return;
    ALLOC_mblock_t *block = _ALLOC_mblock_find(ptr);
    block->isfree = true;

    // cleaning up free blocks from the end of list
    block = ALLOC_mhead->end;
    while (block && !block->nxt && block->isfree) {
        ALLOC_mblock_t *tofree = block;
        ALLOC_mhead->end = block = tofree->prv;
        if (tofree->prv) tofree->prv->nxt = NULL;
        else {
            ALLOC_mhead->start = NULL;
            ALLOC_mhead->end = NULL;
        }
        ALLOC_mhead->blockc--;
        brk(tofree);
    }
}
