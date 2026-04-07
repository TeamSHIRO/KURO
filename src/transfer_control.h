#ifndef TRANSFER_CONTROL_H
#define TRANSFER_CONTROL_H

#include "boot.h"
#include "efi.h"

__attribute__((sysv_abi)) extern _Noreturn void transfer_control(KuroExecutableInfo *exec_info, EFI_HANDLE image_handle,
                                                                 const EFI_SYSTEM_TABLE *system_table, void *data,
                                                                 char *boot_id, uint64_t stack_start,
                                                                 uint64_t entry_point);

#endif // TRANSFER_CONTROL_H
