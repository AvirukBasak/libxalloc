#ifndef __ALLOC_H__
#define __ALLOC_H__

#include <stdlib.h>

/** alloctes specified size */
void *alloc_m(size_t size);

/** resizes allocated block if possible, or copies data around */
void *alloc_re(void *ptr, size_t size);

/** marks pointer to block for cleanup */
void alloc_free(void *ptr);

#endif
