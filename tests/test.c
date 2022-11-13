#include <stdio.h>

#include "liballoc.h"

int main(int argc, char *argv[])
{
    char *s = alloc_m(33);
    for (int i = 0; i < 26; i++) {
        s[i] = i + 0x61;
    }
    printf("%s\n", s);
    alloc_free(s);
    alloc_forceclean();
    return 0;
}
