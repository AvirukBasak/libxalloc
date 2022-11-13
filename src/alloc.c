#include <sys/types.h>
#include <sys/mman.h>
#include <err.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include "alloc.h"

#ifndef __ALLOC_H__CLEANUP_TRIGGER_COUNT__
#define __ALLOC_H__CLEANUP_TRIGGER_COUNT__ (4096)
#endif

typedef struct __ALLOC_H__mhead_st __ALLOC_H__mhead_t;
typedef struct __ALLOC_H__membloc_st __ALLOC_H__membloc_t;

/* functions */
void __ALLOC_H__clean();
__ALLOC_H__membloc_t *__ALLOC_H__mblock_find(void *ptr);

/** head of linked list */
struct __ALLOC_H__mhead_st
{
    __ALLOC_H__membloc_t *start;
    __ALLOC_H__membloc_t *end;
    size_t blockc;
};

/** data of a memory block */
struct __ALLOC_H__membloc_st
{
    __ALLOC_H__membloc_t *prv;
    void *ptr;
    size_t size;
    bool free;
    __ALLOC_H__membloc_t *nxt;
};

/** linked list of block data */
__ALLOC_H__mhead_t *__ALLOC_H__memory = NULL;

/** cleanup trigger count; when count reaches 0, cleanup function is called; count is then reset */
size_t __ALLOC_H__triggerc = __ALLOC_H__CLEANUP_TRIGGER_COUNT__ +1;

/** cleans up heap and if possible reduces heap break point */
void __ALLOC_H__clean()
{}

/** searches for a specific block data based on its address */
__ALLOC_H__membloc_t *__ALLOC_H__mblock_find(void *ptr)
{
    return NULL;
}

/** alloctes specified size */
void *alloc_m(size_t size)
{
    if (!__ALLOC_H__memory) {
        __ALLOC_H__memory = mmap(NULL, sizeof(__ALLOC_H__mhead_t), PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
        __ALLOC_H__memory->blockc = 0;
        __ALLOC_H__memory->start = NULL;
        __ALLOC_H__memory->end = NULL;
    }
    void *ptr = sbrk(size);
    if (ptr == (void *) -1) abort();
    __ALLOC_H__membloc_t *allocator = mmap(NULL, sizeof(__ALLOC_H__membloc_t), PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
    if (allocator == MAP_FAILED) abort();
    allocator->ptr = ptr;
    allocator->size = size;
    allocator->free = false;
    allocator->nxt = __ALLOC_H__memory->start;
    allocator->prv = NULL;
    __ALLOC_H__memory->start = allocator;
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
    __ALLOC_H__membloc_t *block = __ALLOC_H__mblock_find(ptr);
    block->free = true;
    __ALLOC_H__triggerc--;
    if (!__ALLOC_H__triggerc) __ALLOC_H__clean();
    __ALLOC_H__triggerc = __ALLOC_H__CLEANUP_TRIGGER_COUNT__;
}
