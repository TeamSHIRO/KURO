#ifndef FILE_H
#define FILE_H

#include "efi.h"
#include "protocol/efi-fp.h"

extern const EFI_GUID LIP_GUID;
extern const EFI_GUID SFSP_GUID;
extern const EFI_GUID FI_ID;

EFI_STATUS cached_volume_open(EFI_HANDLE image_handle, const EFI_SYSTEM_TABLE *system_table, EFI_FILE_PROTOCOL **output);
EFI_STATUS get_file_size(const EFI_SYSTEM_TABLE *system_table, EFI_FILE_PROTOCOL *file_protocol, UINTN *output);

#endif // FILE_H
