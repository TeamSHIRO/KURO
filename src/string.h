#ifndef STRING_H
#define STRING_H

#include <efi.h>
#include <stddef.h>

#define HEX_BUFFER_SIZE 19
#define STR_BUFFER_SIZE 21

void strncpy(char *dest, const char *src, size_t n);
void wstrncpy(CHAR16 *dest, const CHAR16 *src, size_t n);
size_t strlen(const char *str);
size_t wstrlen(const CHAR16 *str);
void to_wchar(const char *src, CHAR16 *dest, size_t max_len);
void to_char(const CHAR16 *src, char *dest, size_t max_len);
void clean_newline(char *str, size_t max_len);
void to_hex(uint64_t value, CHAR16 *dest);
void to_hex_char(uint64_t value, char *dest);
void to_str(uint64_t value, CHAR16 *buffer);
void to_str_char(uint64_t value, char *buffer);

void *memset(void *dest, int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
size_t memcmp(const void *s1, const void *s2, size_t n);

#endif // STRING_H
