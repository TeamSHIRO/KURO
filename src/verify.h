#ifndef VERIFY_H
#define VERIFY_H

#include <stddef.h>

#include "status.h"

KuroStatus verify_footer(unsigned char *file, size_t FILE_SIZE, const uint8_t public_key[32]);

#endif // VERIFY_H
