#ifndef STRING_H
#define STRING_H

#include <efi.h>
#include <stddef.h>

#define HEX_BUFFER_SIZE 19
#define STR_BUFFER_SIZE 21

void to_wchar(const char *src, CHAR16 *dest, size_t max_len);
void to_hex(uint64_t value, CHAR16 *dest);
void to_hex_char(uint64_t value, char *dest);
void to_str(uint64_t value, CHAR16 *buffer);
void to_str_char(uint64_t value, char *buffer);

#endif // STRING_H
