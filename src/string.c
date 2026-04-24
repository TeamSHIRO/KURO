#include "string.h"

#include <protocol/efi-fp.h>
#include <stddef.h>

void to_wchar(const char *src, CHAR16 *dest, size_t max_len) {
    size_t i = 0;
    while (src[i] != '\0' && i < max_len - 1) {
        dest[i] = (CHAR16) src[i];
        i++;
    }
    dest[i] = L'\0';
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
