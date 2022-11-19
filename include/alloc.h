#ifndef __ALLOC_H__
#define __ALLOC_H__

#include <stdlib.h>

typedef void *ptr_t;

/** alloctes specified size */
ptr_t allocm(size_t size);

/** resizes allocated block if possible, or copies data around */
ptr_t allocre(ptr_t ptr, size_t size);

/** marks pointer to block for cleanup */
void alloc_free(ptr_t ptr);

#endif
