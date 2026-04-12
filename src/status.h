#ifndef ERROR_H
#define ERROR_H

#include "efi.h"

enum KuroStatus {
    Watchdog_DisableFailed,

    Elf_FileNotFound,
    Elf_Unreadable,
    Elf_FailedSetPos,
    Elf_ContainsRelocation,
    Elf_InvalidHeader,
    Elf_InvalidProgramHeader,
    Elf_InvalidMemorySize,

    Load_Failed,

    Framebuffer_ModeFetchFailed,
    Framebuffer_PrepareFailed,

    System_OutOfMemory,
    System_AllocationFailed,
    System_ExitBootServicesFailed,
    System_CannotOpenVolume,

    Unknown,
    Success
};

typedef struct {
    EFI_STATUS error_code; // This one tells us what error code is it exactly
    enum KuroStatus status;
} ErrorStatus;

const CHAR16 *status_to_str(ErrorStatus status);
void throw_error(EFI_SYSTEM_TABLE *system_table, ErrorStatus error);

#endif // ERROR_H
