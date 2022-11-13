#ifndef __ALLOC_H__
#define __ALLOC_H__

#include <stdlib.h>

void *alloc_m(size_t size);
void *alloc_re(void *ptr, size_t size);
void alloc_free(void *ptr);

#endif
