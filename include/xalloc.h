#ifndef __XALLOC_H__
#define __XALLOC_H__

#include "stdhead.h"

/**
 * @brief Alloctes specified size
 * @param size Size of allocation
 * @return ptr_t Pointer to new allocation
 */
ptr_t xmalloc(size_t size);

/**
 * @brief Alloctes specified size and initializes with 0
 * @param count Count of allocations
 * @param size Size of 1 allocation
 * @return ptr_t Pointer to new allocation
 */
ptr_t xcalloc(size_t count, size_t size);

/**
 * @brief Reallocates an allocated bloc
 * Resizes allocated block if possible, or copies data around.
 * @param ptr Pointer to allocated bloc or NULL
 * @param size Size of re-allocation
 * @return ptr_t Pointer to new re-allocation
 */
ptr_t xrealloc(ptr_t ptr, size_t size);

/**
 * @brief Marks pointer to block for cleanup
 * @param ptr Pointer to be freed
 * @return size_t Size actually freed using brk
 */
size_t xfree(ptr_t ptr);

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
