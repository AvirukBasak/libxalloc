#ifndef __XALLOC_H__
#define __XALLOC_H__

#include <stdlib.h>

typedef void *ptr_t;

/** alloctes specified size */
ptr_t xmalloc(size_t size);

/** resizes allocated block if possible, or copies data around */
ptr_t xrealloc(ptr_t ptr, size_t size);

/** marks pointer to block for cleanup */
void xfree(ptr_t ptr);

#endif
