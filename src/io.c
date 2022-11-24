#include <unistd.h>

#include "stdhead.h"
#include "stdfunc.h"
#include "io.h"

void _xalloc_print_str(int fd, const char *s)
{
    if (!s) return;
    size_t len = strlen(s);
    write(fd, s, len);
}

void _xalloc_print_ptr(int fd, const ptr_t p)
{
    char *b = (char*)(&p);
    char s[16];
    bool is_bend = _xalloc_std_is_litle_endian();
    int i = is_bend ? 7 : 0;
    for (int j = 0; (is_bend ? (i >= 0) : (i < 8)) && j < 16; j+=2) {
        const ui8_t byte = b[i];
        s[j] = _xalloc_std_to_hex(byte >> 4);
        s[j+1] = _xalloc_std_to_hex(byte);
        is_bend ? i-- : i++;
    }
    write(fd, "0x", 2);
    write(fd, s, 16);
}
