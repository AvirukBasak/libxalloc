#ifndef __LIBXALLOC_H__
#define __LIBXALLOC_H__

#include <stddef.h>

/** alloctes specified size */
void *xmalloc(size_t size);

/** resizes allocated block if possible, or copies data around */
void *xrealloc(void *ptr, size_t size);

/** marks pointer to block for cleanup */
void *xfree(void *ptr);

#endif
