#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "xalloc.h"

#define MAX(a,b) (a>b?a:b)
#define MIN(a,b) (a<b?a:b)

#define COPY_THRESHOLD (4096)
#define MBLOC_PADDING  (16)

#define NULLPTR_CHECK(ptr) if (!ptr || ptr == (void*) -1) _xalloc_abort("null pointer")

typedef struct XALLOC_mhead_t XALLOC_mhead_t;
typedef struct XALLOC_mbloc_t XALLOC_mbloc_t;

/** bloc data list head allocated in global memory */
XALLOC_mhead_t _XALLOC_mhead;
XALLOC_mhead_t *XALLOC_mhead = NULL;

/* functions */
void _xalloc_abort(const char *s);
void _xalloc_mhead_init();
void _xalloc_mbloc_link(XALLOC_mbloc_t *node);
XALLOC_mbloc_t *_xalloc_mbloc_new(size_t size);
XALLOC_mbloc_t *_xalloc_mbloc_find(ptr_t ptr);
XALLOC_mbloc_t *_xalloc_mbloc_split(XALLOC_mbloc_t *bloc, size_t req_sz);
XALLOC_mbloc_t *_xalloc_mbloc_merge(XALLOC_mbloc_t *bloc, size_t req_sz);

/** head of linked list */
struct XALLOC_mhead_t
{
    size_t blocc;
    XALLOC_mbloc_t *start;
    XALLOC_mbloc_t *end;
};

/** data of a memory bloc */
struct XALLOC_mbloc_t
{
    char padding[MBLOC_PADDING];
    bool isfree;
    ptr_t ptr;
    size_t size;
    XALLOC_mbloc_t *prv;
    XALLOC_mbloc_t *nxt;
};

/** Abort with error message */
void _xalloc_abort(const char *s)
{
    NULLPTR_CHECK(s);
    size_t len = strlen(s);
    write(2, "libxalloc: aborted: ", 20);
    write(2, s, len);
    write(2, "\n", 1);
    abort();
}

void _xalloc_mhead_init()
{
    if (XALLOC_mhead) return;
    XALLOC_mhead = &_XALLOC_mhead;
    XALLOC_mhead->blocc = 0;
    XALLOC_mhead->start = NULL;
    XALLOC_mhead->end = NULL;
}

void _xalloc_mbloc_link(XALLOC_mbloc_t *node)
{
    NULLPTR_CHECK(node);
    // setting last node links
    if (XALLOC_mhead->end)
        XALLOC_mhead->end->nxt = node;
    // setting new node links
    node->prv = XALLOC_mhead->end;
    node->nxt = NULL;
    // updating meta data at head
    if (!XALLOC_mhead->start)
        XALLOC_mhead->start = node;
    XALLOC_mhead->end = node;
    XALLOC_mhead->blocc++;
}

XALLOC_mbloc_t *_xalloc_mbloc_new(size_t size)
{
    XALLOC_mbloc_t *node = sbrk(sizeof(XALLOC_mbloc_t));
    NULLPTR_CHECK(node);
    ptr_t ptr = sbrk(size);
    NULLPTR_CHECK(ptr);
    node->ptr = ptr;
    node->size = size;
    node->isfree = false;
    _xalloc_mbloc_link(node);
    return node;
}

/** searches for a specific bloc data based on its address */
XALLOC_mbloc_t *_xalloc_mbloc_find(ptr_t ptr)
{
    NULLPTR_CHECK(ptr);
    XALLOC_mbloc_t *p = XALLOC_mhead->start;
    while (p) {
        if (p->ptr == ptr) return p;
        p = p->nxt;
    }
    _xalloc_abort("invalid pointer");
    return NULL;
}

XALLOC_mbloc_t *_xalloc_mbloc_split(XALLOC_mbloc_t *bloc, size_t req_sz)
{
    NULLPTR_CHECK(bloc);
    if (req_sz == bloc->size) return bloc;
    if (req_sz > bloc->size) _xalloc_abort("post split size exceeds bloc size");
    size_t leftover_sz = bloc->size - req_sz - sizeof(XALLOC_mbloc_t);
    /* if remaining memory is less-equal double the size of a memory head,
     * then no changes are made
     */
    if (leftover_sz <= 2 * sizeof(XALLOC_mbloc_t)) return bloc;
    XALLOC_mbloc_t *leftover = (XALLOC_mbloc_t*) (bloc->ptr + req_sz);
    leftover->isfree = true;
    leftover->ptr = (ptr_t) (leftover + sizeof(XALLOC_mbloc_t));
    leftover->size = leftover_sz;
    leftover->prv = bloc;
    leftover->nxt = bloc->nxt;
    bloc->nxt = leftover;
    bloc->size = req_sz;
    return bloc;
}

XALLOC_mbloc_t *_xalloc_mbloc_merge(XALLOC_mbloc_t *bloc, size_t req_sz)
{
    NULLPTR_CHECK(bloc);
    if (req_sz == bloc->size) return bloc;
    if (bloc->size > req_sz) _xalloc_abort("bloc size exceeds post merge size");
    size_t avlb_sz = bloc->size;
    XALLOC_mbloc_t *node = bloc->nxt;
    while (avlb_sz < req_sz && node && node->isfree) {
        avlb_sz += node->size + sizeof(XALLOC_mbloc_t);
        if (avlb_sz < req_sz) node = node->nxt;
        else break;
    }
    if (avlb_sz < req_sz)
        return NULL;
    if (avlb_sz > req_sz)
        node = _xalloc_mbloc_split(node, node->size - (avlb_sz - req_sz));
    if (node->nxt) node->nxt->prv = bloc;
    bloc->nxt = node->nxt;
    bloc->size = req_sz;
    return bloc;
}

/** alloctes specified size */
ptr_t xmalloc(size_t size)
{
    if (!XALLOC_mhead)
        _xalloc_mhead_init();

    // attempting to recycle old empty bloc
    if (XALLOC_mhead->start) {
        XALLOC_mbloc_t *reusable = XALLOC_mhead->start;
        while (reusable)
            if (reusable->isfree && reusable->size >= size) break;
            else reusable = reusable->nxt;
        if (reusable) {
            reusable->isfree = false;
            if (reusable->size == size) return reusable->ptr;
            return _xalloc_mbloc_split(reusable, size)->ptr;
        }
    }

    // fallback: allocating new bloc
    return _xalloc_mbloc_new(size)->ptr;
}

/** resizes allocated bloc if possible, or copies data around */
ptr_t xrealloc(ptr_t ptr, size_t size)
{
    if (!ptr) xmalloc(size);
    XALLOC_mbloc_t *bloc = _xalloc_mbloc_find(ptr);
    if (bloc->size == size) return ptr;

    // splitting blocs if new size is smaller
    if (size < bloc->size)
        return _xalloc_mbloc_split(bloc, size)->ptr;

    // if bloc is too large
    if (bloc->size > COPY_THRESHOLD) {
        // last bloc: update brk
        if (!bloc->nxt) {
            sbrk(size - bloc->size);
            bloc->size = size;
            return ptr;
        }
        // consecutive empty blocs
        else if (bloc->nxt->isfree) {
            XALLOC_mbloc_t *merged = _xalloc_mbloc_merge(bloc, size);
            if (merged) return merged->ptr;
        }
    }

    // fallback: new bloc allocation and copy data
    ptr_t newptr = xmalloc(size);
    memcpy(newptr, bloc->ptr, MIN(bloc->size, size));
    bloc->isfree = true;
    return newptr;
}

/** marks pointer to bloc for cleanup */
void xfree(ptr_t ptr)
{
    if (!ptr) return;
    XALLOC_mbloc_t *bloc = _xalloc_mbloc_find(ptr);
    bloc->isfree = true;

    // cleaning up free blocs from the end of list
    bloc = XALLOC_mhead->end;
    while (bloc && !bloc->nxt && bloc->isfree) {
        XALLOC_mbloc_t *tofree = bloc;
        XALLOC_mhead->end = bloc = tofree->prv;
        if (tofree->prv) tofree->prv->nxt = NULL;
        else {
            XALLOC_mhead->start = NULL;
            XALLOC_mhead->end = NULL;
        }
        XALLOC_mhead->blocc--;
        brk(tofree);
    }
}
