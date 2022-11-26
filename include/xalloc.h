#ifndef __XALLOC_H__
#define __XALLOC_H__

#include "stdhead.h"

/** alloctes specified size */
ptr_t xmalloc(size_t size);

/** resizes allocated block if possible, or copies data around */
ptr_t xrealloc(ptr_t ptr, size_t size);

/** marks pointer to block for cleanup */
void xfree(ptr_t ptr);

#define COPY_THRESHOLD (4096)
#define MBLOC_PADDING  (15)

#define NULLPTR_CHECK(ptr) if (!ptr || ptr == (void*) -1) __xalloc_abort("null pointer")

typedef struct XALLOC_mhead_t XALLOC_mhead_t;
typedef struct XALLOC_mbloc_t XALLOC_mbloc_t;

/* functions */
void __xalloc_abort(const char *s);
void __xalloc_mhead_init();
bool __xalloc_integrity_verify();
void __xalloc_mbloc_link(XALLOC_mbloc_t *node);
XALLOC_mbloc_t *__xalloc_mbloc_new(size_t size);
XALLOC_mbloc_t *__xalloc_mbloc_find(ptr_t ptr);
XALLOC_mbloc_t *__xalloc_mbloc_split(XALLOC_mbloc_t *bloc, size_t req_sz);
XALLOC_mbloc_t *__xalloc_mbloc_merge(XALLOC_mbloc_t *bloc, size_t req_sz);

#endif
