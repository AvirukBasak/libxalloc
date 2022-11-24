#include <unistd.h>

#include "stdhead.h"
#include "io.h"

void print_str(int fd, const char *s)
{
    if (!s) return;
    size_t len = strlen(s);
    write(fd, s, len);
}

char _print_to_hex(const ui8_t _4bits)
{
    const ui8_t bits4 = _4bits & 0x0f;
    if (bits4 >= 0x00 && bits4 <= 0x09) return bits4 + '0';
    else if (bits4 >= 0x0a && bits4 <= 0x0f) return bits4 - 0x0a + 'a';
    else abort();
}

void print_ptr(int fd, const ptr_t p)
{
    char *b = (char*)(&p);
    char s[16];
    for (int i = 0, j = 0; i < 8 && j < 16; i++, j+=2) {
        const ui8_t byte = b[i];
        s[j] = _print_to_hex(byte >> 4);
        s[j+1] = _print_to_hex(byte);
    }
    write(fd, "0x", 2);
    write(fd, s, 16);
}
