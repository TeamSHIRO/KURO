#ifndef TRANSFER_CONTROL_H
#define TRANSFER_CONTROL_H

#include "efi.h"
#include "boot.h"

extern EFI_HANDLE transfer_image_handle;
extern EFI_SYSTEM_TABLE* transfer_system_table;
extern KuroExecutableInfo* transfer_exec_info;
extern char* transfer_boot_id_addr;
extern uint64_t transfer_stack_start;

extern uint64_t transfer_entry_point;

extern _Noreturn void transfer_control(void);

#endif //TRANSFER_CONTROL_H
