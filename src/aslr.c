#include "aslr.h"

#include "boot.h"
#include "efi_helper.h"

EFI_STATUS alloc_prog(size_t page_needed, AslrConfig aslr_enabled, EFI_PHYSICAL_ADDRESS *output) {
    *output = 0;
    if (aslr_enabled == ASLR_DISABLED) {

    }
    return EFI_ERR(EFI_UNSUPPORTED); // TODO: mono - Add ASLR
}
