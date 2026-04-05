#ifndef STRING_H
#define STRING_H

#include <efi.h>
#include <stddef.h>

void wchar(const char *src, CHAR16 *dest, size_t max_len);
void to_hex(uint64_t value, CHAR16 *dest);
void to_str(uint64_t value, char *buffer);

#endif // STRING_H
