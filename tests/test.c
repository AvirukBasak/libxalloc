#include <stdio.h>

#include "liballoc.h"

int main(int argc, char *argv[])
{
    for (int i = 0; i < 7; i++) {
        char *s = allocm(33);
        for (int i = 0; i < 26; i++) {
            s[i] = i + 0x61;
        }
        printf("%d: %s\n", i, s);
        alloc_free(s);
    }
    return 0;
}
