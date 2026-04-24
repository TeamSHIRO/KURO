#include "memory.h"

#include <stddef.h>

void *memset(void *dest, int c, size_t n) {
    unsigned char *p = dest;
    while (n--) {
        *p++ = (unsigned char) c;
    }
    return dest;
}

void *memcpy(void *dest, const void *src, size_t n) {
    unsigned char *d = dest;
    const unsigned char *s2 = src;
    while (n--) {
        *d++ = *s2++;
    }
    return dest;
}
