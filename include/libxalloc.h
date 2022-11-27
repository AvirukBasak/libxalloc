#ifndef __LIBXALLOC_H__
#define __LIBXALLOC_H__

#include <stddef.h>

/**
 * @brief Alloctes specified size
 * @param size Size of allocation
 * @return Pointer to new allocation
 */
void *xmalloc(size_t size);

/**
 * @brief Reallocates an allocated bloc
 * Resizes allocated block if possible, or copies data around.
 * @param ptr Pointer to allocated bloc or NULL
 * @param size Size of re-allocation
 * @return void* Pointer to new re-allocation
 */
void *xrealloc(void *ptr, size_t size);

/**
 * @brief Marks pointer to block for cleanup
 * @param ptr Pointer to be freed
 * @return size_t Size actually freed using brk
 */
size_t xfree(void *ptr);

#endif
