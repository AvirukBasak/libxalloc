#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "alloc.h"

#define MAX(a,b) (a>b?a:b)
#define MIN(a,b) (a<b?a:b)

#define ALLOC_COPY_THRESHOLD (4096)
#define ALLOC_MBLOC_PADDING  (16)

#define ALLOC_NULLCHECK(ptr) if (!ptr || ptr == (void*) -1) _alloc_abort("null pointer")

typedef struct ALLOC_mhead_t ALLOC_mhead_t;
typedef struct ALLOC_mbloc_t ALLOC_mbloc_t;

/** linked list of bloc data */
ALLOC_mhead_t *ALLOC_mhead = NULL;

/* functions */
void _alloc_abort(const char *s);
void _alloc_mhead_allocate();
void _alloc_mbloc_link(ALLOC_mbloc_t *node);
ALLOC_mbloc_t *_alloc_mbloc_new(size_t size);
ALLOC_mbloc_t *_alloc_mbloc_find(ptr_t ptr);
ALLOC_mbloc_t *_alloc_mbloc_split(ALLOC_mbloc_t *bloc, size_t req_sz);
ALLOC_mbloc_t *_alloc_mbloc_merge(ALLOC_mbloc_t *bloc, size_t req_sz);

/** head of linked list */
struct ALLOC_mhead_t
{
    size_t blocc;
    ALLOC_mbloc_t *start;
    ALLOC_mbloc_t *end;
};

/** data of a memory bloc */
struct ALLOC_mbloc_t
{
    char padding[ALLOC_MBLOC_PADDING];
    bool isfree;
    ptr_t ptr;
    size_t size;
    ALLOC_mbloc_t *prv;
    ALLOC_mbloc_t *nxt;
};

/** Abort with error message */
void _alloc_abort(const char *s)
{
    size_t len = strlen(s);
    write(2, "liballoc: aborted: ", 19);
    write(2, s, len);
    write(2, "\n", 1);
    abort();
}

void _alloc_mhead_allocate()
{
    if (ALLOC_mhead) return;
    ALLOC_mhead = sbrk(sizeof(ALLOC_mhead_t));
    ALLOC_NULLCHECK(ALLOC_mhead);
    ALLOC_mhead->blocc = 0;
    ALLOC_mhead->start = NULL;
    ALLOC_mhead->end = NULL;
}

void _alloc_mbloc_link(ALLOC_mbloc_t *node)
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
    ALLOC_mhead->blocc++;
}

ALLOC_mbloc_t *_alloc_mbloc_new(size_t size)
{
    ALLOC_mbloc_t *node = sbrk(sizeof(ALLOC_mbloc_t));
    ALLOC_NULLCHECK(node);
    ptr_t ptr = sbrk(size);
    ALLOC_NULLCHECK(ptr);
    node->ptr = ptr;
    node->size = size;
    node->isfree = false;
    _alloc_mbloc_link(node);
    return node;
}

/** searches for a specific bloc data based on its address */
ALLOC_mbloc_t *_alloc_mbloc_find(ptr_t ptr)
{
    ALLOC_mbloc_t *p = ALLOC_mhead->start;
    while (p) {
        if (p->ptr == ptr) return p;
        p = p->nxt;
    }
    _alloc_abort("invalid pointer");
    return NULL;
}

ALLOC_mbloc_t *_alloc_mbloc_split(ALLOC_mbloc_t *bloc, size_t req_sz)
{
    ALLOC_NULLCHECK(bloc);
    if (req_sz == bloc->size) return bloc;
    if (req_sz > bloc->size) _alloc_abort("size post split exceeds available size");
    size_t leftover_sz = bloc->size - req_sz - sizeof(ALLOC_mbloc_t);
    /* if remaining memory is less-equal double the size of a memory head,
     * then no changes are made
     */
    if (leftover_sz <= 2 * sizeof(ALLOC_mbloc_t)) return bloc;
    ALLOC_mbloc_t *leftover = (ALLOC_mbloc_t*) (bloc->ptr + req_sz);
    leftover->isfree = true;
    leftover->ptr = (ptr_t) (leftover + sizeof(ALLOC_mbloc_t));
    leftover->size = leftover_sz;
    leftover->prv = bloc;
    leftover->nxt = bloc->nxt;
    bloc->nxt = leftover;
    bloc->size = req_sz;
    return bloc;
}

ALLOC_mbloc_t *_alloc_mbloc_merge(ALLOC_mbloc_t *bloc, size_t req_sz)
{
    ALLOC_NULLCHECK(bloc);
    if (req_sz <= bloc->size) return bloc;
    size_t avlb_sz = bloc->size;
    ALLOC_mbloc_t *node = bloc->nxt;
    while (avlb_sz < req_sz && node && node->isfree) {
        avlb_sz += node->size;
        node = node->nxt;
    }
    if (avlb_sz < req_sz)
        return NULL;
    if (avlb_sz > req_sz) {
        node = _alloc_mbloc_split(node, node->size - (avlb_sz - req_sz));
        node->nxt->prv = bloc;
    }
    bloc->nxt = node->nxt;
    bloc->size = req_sz;
    return bloc;
}

/** alloctes specified size */
ptr_t allocm(size_t size)
{
    if (!ALLOC_mhead)
        _alloc_mhead_allocate();

    // attempting to recycle old empty bloc
    if (ALLOC_mhead->start) {
        ALLOC_mbloc_t *reusable = ALLOC_mhead->start;
        while (reusable)
            if (reusable->isfree && reusable->size >= size) break;
            else reusable = reusable->nxt;
        if (reusable) {
            reusable->isfree = false;
            if (reusable->size == size) return reusable->ptr;
            return _alloc_mbloc_split(reusable, size)->ptr;
        }
    }

    // fallback: allocating new bloc
    return _alloc_mbloc_new(size)->ptr;
}

/** resizes allocated bloc if possible, or copies data around */
ptr_t allocre(ptr_t ptr, size_t size)
{
    if (!ptr) allocm(size);
    ALLOC_mbloc_t *bloc = _alloc_mbloc_find(ptr);
    if (bloc->size == size) return ptr;

    // splitting blocs if new size is smaller
    if (size < bloc->size)
        return _alloc_mbloc_split(bloc, size)->ptr;

    // if bloc is too large
    if (bloc->size > ALLOC_COPY_THRESHOLD) {
        // last bloc: update brk
        if (!bloc->nxt) {
            sbrk(size - bloc->size);
            bloc->size = size;
            return ptr;
        }
        // consecutive empty blocs
        else if (bloc->nxt->isfree) {
            ALLOC_mbloc_t *merged = _alloc_mbloc_merge(bloc, size);
            if (merged) return merged->ptr;
        }
    }

    // fallback: new bloc allocation and copy data
    ptr_t newptr = allocm(size);
    memcpy(newptr, bloc->ptr, MIN(bloc->size, size));
    bloc->isfree = true;
    return newptr;
}

/** marks pointer to bloc for cleanup */
void alloc_free(ptr_t ptr)
{
    if (!ptr) return;
    ALLOC_mbloc_t *bloc = _alloc_mbloc_find(ptr);
    bloc->isfree = true;

    // cleaning up free blocs from the end of list
    bloc = ALLOC_mhead->end;
    while (bloc && !bloc->nxt && bloc->isfree) {
        ALLOC_mbloc_t *tofree = bloc;
        ALLOC_mhead->end = bloc = tofree->prv;
        if (tofree->prv) tofree->prv->nxt = NULL;
        else {
            ALLOC_mhead->start = NULL;
            ALLOC_mhead->end = NULL;
        }
        ALLOC_mhead->blocc--;
        brk(tofree);
    }
}
