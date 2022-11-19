#include <stdio.h>
#include <unistd.h>

#include "libxalloc.h"

int main(int argc, char *argv[])
{
    void *p0, *p1;
    printf("brk init = %p\n", p0 = sbrk(0));
    for (int i = 0; i < 7; i++) {
        char *s = xmalloc(256*1024*1024);
        for (int i = 0; i < 26; i++) {
            s[i] = i + 0x61;
        }
        char *st0 = xmalloc(256*1024*1024);
        for (int i = 0; i < 26; i++) {
            st0[i] = i + 0x61;
        }
        char *st1 = xmalloc(256*1024*1024);
        for (int i = 0; i < 26; i++) {
            st1[i] = i + 0x61;
        }
        char *st2 = xmalloc(256*1024*1024);
        for (int i = 0; i < 26; i++) {
            st2[i] = i + 0x61;
        }
        printf("%d: %s\n", i, st2);
        xfree(s);
        xfree(st0);
        xfree(st1);
        xfree(st2);
    }
    printf("brk exit = %p\n", p1 = sbrk(0));
    printf("brk difference = %zu B\n", (size_t) (p1 - p0));
    return 0;
}
