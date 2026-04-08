#ifndef FILE_H
#define FILE_H

#include "efi.h"
#include "protocol/efi-fp.h"

EFI_STATUS volume_open(EFI_HANDLE image_handle, const EFI_SYSTEM_TABLE *system_table, EFI_FILE_PROTOCOL **output);
EFI_STATUS get_file_size(const EFI_SYSTEM_TABLE *system_table, const EFI_FILE_PROTOCOL *file_protocol, UINTN *output);

#endif // FILE_H
