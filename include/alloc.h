#ifndef __ALLOC_H__
#define __ALLOC_H__

#include <stdlib.h>

typedef void *pointer_t;

/** alloctes specified size */
pointer_t allocm(size_t size);

/** resizes allocated block if possible, or copies data around */
pointer_t allocre(pointer_t ptr, size_t size);

/** marks pointer to block for cleanup */
void alloc_free(pointer_t ptr);

#endif
