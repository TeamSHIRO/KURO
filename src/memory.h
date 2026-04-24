#ifndef MEMORY_H
#define MEMORY_H
#include <stddef.h>

void *memset(void *dest, int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);

#endif // MEMORY_H
