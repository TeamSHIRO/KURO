#ifndef ASLR_H
#define ASLR_H

#include <stddef.h>

#include "efi-st.h"

typedef enum {
    ASLR_DISABLED = 0,
    ASLR_ENABLED = 1
} AslrConfig;

EFI_STATUS alloc_prog(size_t page_needed, AslrConfig aslr_enabled, EFI_PHYSICAL_ADDRESS* output);

#endif //ASLR_H
