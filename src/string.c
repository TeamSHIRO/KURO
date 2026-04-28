#include "string.h"

#include <protocol/efi-fp.h>
#include <stddef.h>

void strncpy(char *dest, const char *src, size_t n) {
    size_t i = 0;
    while (i < n && src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    if (i < n) {
        dest[i] = '\0';
    }
}

void wstrncpy(CHAR16 *dest, const CHAR16 *src, size_t n) {
    size_t i = 0;
    while (i < n && src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    if (i < n) {
        dest[i] = '\0';
    }
}

int strlen(const char *str) {
    int i = 0;
    while (str[i] != '\0') {
        i++;
    }
    return i;
}

int wstrlen(const CHAR16 *str) {
    int i = 0;
    while (str[i] != '\0') {
        i++;
    }
    return i;
}

void to_wchar(const char *src, CHAR16 *dest, size_t max_len) {
    size_t i = 0;
    while (i + 1 < max_len && src[i] != 0) {
        dest[i] = (CHAR16) src[i];
        i++;
    }
    dest[i] = L'\0';
}

void to_char(const CHAR16 *src, char *dest, size_t max_len) {
    size_t i = 0;
    while (i + 1 < max_len && src[i] != 0) {
        dest[i] = (char) src[i];
        i++;
    }
    dest[i] = '\0';
}

void clean_newline(char *str, size_t max_len) {
    size_t i = 0;
    while (i + 1 < max_len && str[i] != 0) {
        if (str[i + 1] == '\0' && str[i] == '\n') {
            str[i] = '\0';
            return;
        }
        if (str[i] == '\r') {
            str[i] = str[i + 1];
        }
        i++;
    }
}

static const char HEX_DIGITS[16] = "0123456789ABCDEF";

void to_hex(uint64_t value, CHAR16 *dest) {
    dest[0] = '0';
    dest[1] = 'x';

    if (value == 0) {
        dest[2] = '0';
        dest[3] = '\0';
        return;
    }

    CHAR16 temp[16];
    size_t i = 0;

    while (value != 0) {
        temp[i++] = (CHAR16) HEX_DIGITS[value & 0xF];
        value >>= 4;
    }

    for (size_t j = 0; j < i; j++) {
        dest[2 + j] = temp[i - 1 - j];
    }

    dest[2 + i] = '\0';
}

void to_hex_char(uint64_t value, char *dest) {
    dest[0] = '0';
    dest[1] = 'x';

    if (value == 0) {
        dest[2] = '0';
        dest[3] = '\0';
        return;
    }

    char temp[16];
    size_t i = 0;

    while (value != 0) {
        temp[i++] = HEX_DIGITS[value & 0xF];
        value >>= 4;
    }

    for (size_t j = 0; j < i; j++) {
        dest[2 + j] = temp[i - 1 - j];
    }

    dest[2 + i] = '\0';
}

void to_str(uint64_t value, CHAR16 *buffer) {
    CHAR16 temp[20];
    int i = 0;

    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }

    while (value > 0) {
        temp[i++] = (CHAR16) ('0' + (uint8_t) (value % 10));
        value /= 10;
    }

    int j = 0;
    while (i > 0) {
        buffer[j++] = temp[--i];
    }

    buffer[j] = '\0';
}

void to_str_char(uint64_t value, char *buffer) {
    char temp[20];
    int i = 0;

    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }

    while (value > 0) {
        temp[i++] = (char) ('0' + (uint8_t) (value % 10));
        value /= 10;
    }

    int j = 0;
    while (i > 0) {
        buffer[j++] = temp[--i];
    }

    buffer[j] = '\0';
}

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

int memcmp(const void *s1, const void *s2, size_t n) {
    const unsigned char *p1 = s1;
    const unsigned char *p2 = s2;
    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] - p2[i];
        }
    }
    return 0;}