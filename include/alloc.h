#ifndef __ALLOC_H__
#define __ALLOC_H__

#include <stdlib.h>

/** alloctes specified size */
void *allocm(size_t size);

/** resizes allocated block if possible, or copies data around */
void *allocre(void *ptr, size_t size);

/** marks pointer to block for cleanup */
void alloc_free(void *ptr);

#endif
