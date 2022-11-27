#include <stdio.h>
#include <unistd.h>

#include "libxalloc.h"
#define SZ (256*1024*1024)

/* These functions comes from libxalloc. Since libxalloc
 * doesn't provide the declarations of these functions, I've
 * put the declarations here.
 * These functions simply prints a string/number, but use Linux
 * system calls directly and don't need to allocate memory, which
 * prevent them from interfering with our memory allocation activities.
 */
void __xalloc_print_str(int fd, const char *s);
void __xalloc_print_ui64(int fd, size_t size);

/* overriding libc allocators with custom functions.
 * note that if printf calls a heap allocator, it'll
 * call one of the following
 */

void *malloc(size_t size)
{
    __xalloc_print_str(2, ">> malloc called with size = '");
    __xalloc_print_ui64(2, size);
    __xalloc_print_str(2, " B'\n");
    return xmalloc(size);
}

void *calloc(size_t count, size_t size)
{
    __xalloc_print_str(2, ">> calloc called with size = '");
    __xalloc_print_ui64(2, size);
    __xalloc_print_str(2, " B'\n");
    void *p = xmalloc(count * size);
    memset(p, 0, count * size);
    return p;
}

void *realloc(void *p, size_t size)
{
    __xalloc_print_str(2, ">> realloc called with size = '");
    __xalloc_print_ui64(2, size);
    __xalloc_print_str(2, " B'\n");
    return xrealloc(p, size);
}

void free(void *p)
{
    __xalloc_print_str(2, ">> free called, freed = '");
    __xalloc_print_ui64(2, xfree(p));
    __xalloc_print_str(2, " B'\n");
}

int main(int argc, char *argv[])
{
    void *p0, *p1;
    printf("brk init = %p\n", p0 = sbrk(0));
    for (int i = 0; i < 7; i++) {
        char *s0 = malloc(SZ);
        for (int i = 0; i < SZ; i++) {
            s0[i] = i%26 + 0x61;
        }
        char *s1 = malloc(SZ);
        for (int i = 0; i < SZ; i++) {
            s1[i] = i%26 + 0x61;
        }
        char *s2 = malloc(SZ);
        for (int i = 0; i < SZ; i++) {
            s2[i] = i%26 + 0x61;
        }
        char *s3 = malloc(SZ);
        for (int i = 0; i < SZ; i++) {
            s3[i] = i%26 + 0x61;
        }
        s3[26] = 0; printf("%d: %s\n", i, s3);
        free(s0);
        free(s1);
        free(s2);
        free(s3);
    }
    printf("brk exit = %p\n", p1 = sbrk(0));
    printf("brk difference = %zu B\n", (size_t) (p1 - p0));
    return 0;
}
