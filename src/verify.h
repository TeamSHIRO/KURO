#ifndef VERIFY_H
#define VERIFY_H

#include <stddef.h>

#include "status.h"
#include "protocol/efi-sfsp.h"

ErrorStatus verify_footer(EFI_FILE_PROTOCOL *file, size_t FILE_SIZE, const uint8_t public_key[32]);

#endif // VERIFY_H
