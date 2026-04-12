#include "status.h"
#include <efi.h>
#include "string.h"

const CHAR16 *status_to_str(ErrorStatus status) {
    enum KuroStatus error = status.status;
    switch (error) {
        case Watchdog_DisableFailed:
            return (CHAR16 *) L"Failed to disable watchdog timer";
        case Elf_FileNotFound:
            return (CHAR16 *) L"ELF file not found";
        case Elf_Unreadable:
            return (CHAR16 *) L"ELF file is unreadable";
        case Elf_FailedSetPos:
            return (CHAR16 *) L"Failed to set position in ELF file";
        case Elf_ContainsRelocation:
            return (CHAR16 *) L"ELF file contains unsupported relocation";
        case Elf_InvalidHeader:
            return (CHAR16 *) L"ELF file has an invalid header";
        case Elf_InvalidProgramHeader:
            return (CHAR16 *) L"ELF file has an invalid program header";
        case Elf_InvalidMemorySize:
            return (CHAR16 *) L"ELF file has an invalid memory size";
        case Load_Failed:
            return (CHAR16 *) L"Failed to load";
        case Framebuffer_ModeFetchFailed:
            return (CHAR16 *) L"Failed to fetch framebuffer mode";
        case Framebuffer_PrepareFailed:
            return (CHAR16 *) L"Failed to prepare framebuffer";
        case System_OutOfMemory:
            return (CHAR16 *) L"System is out of memory";
        case System_AllocationFailed:
            return (CHAR16 *) L"System memory allocation failed";
        case System_ExitBootServicesFailed:
            return (CHAR16 *) L"Failed to exit boot services";
        case System_CannotOpenVolume:
            return (CHAR16 *) L"Cannot open volume";
        case Success:
            return (CHAR16 *) L"Success";
        case Unknown:
        default:
            return (CHAR16 *) L"An unexpected error occurred";
    }
}

const CHAR16 *efi_status_to_str(EFI_STATUS status) {
    switch (status) {
        case EFI_SUCCESS:
            return (CHAR16 *) L"EFI_SUCCESS";
        case EFI_LOAD_ERROR:
            return (CHAR16 *) L"EFI_LOAD_ERROR";
        case EFI_INVALID_PARAMETER:
            return (CHAR16 *) L"EFI_INVALID_PARAMETER";
        case EFI_UNSUPPORTED:
            return (CHAR16 *) L"EFI_UNSUPPORTED";
        case EFI_BAD_BUFFER_SIZE:
            return (CHAR16 *) L"EFI_BAD_BUFFER_SIZE";
        case EFI_BUFFER_TOO_SMALL:
            return (CHAR16 *) L"EFI_BUFFER_TOO_SMALL";
        case EFI_NOT_READY:
            return (CHAR16 *) L"EFI_NOT_READY";
        case EFI_DEVICE_ERROR:
            return (CHAR16 *) L"EFI_DEVICE_ERROR";
        case EFI_WRITE_PROTECTED:
            return (CHAR16 *) L"EFI_WRITE_PROTECTED";
        case EFI_OUT_OF_RESOURCES:
            return (CHAR16 *) L"EFI_OUT_OF_RESOURCES";
        case EFI_VOLUME_CORRUPTED:
            return (CHAR16 *) L"EFI_VOLUME_CORRUPTED";
        case EFI_VOLUME_FULL:
            return (CHAR16 *) L"EFI_VOLUME_FULL";
        case EFI_NO_MEDIA:
            return (CHAR16 *) L"EFI_NO_MEDIA";
        case EFI_MEDIA_CHANGED:
            return (CHAR16 *) L"EFI_MEDIA_CHANGED";
        case EFI_NOT_FOUND:
            return (CHAR16 *) L"EFI_NOT_FOUND";
        case EFI_ACCESS_DENIED:
            return (CHAR16 *) L"EFI_ACCESS_DENIED";
        case EFI_NO_RESPONSE:
            return (CHAR16 *) L"EFI_NO_RESPONSE";
        case EFI_NO_MAPPING:
            return (CHAR16 *) L"EFI_NO_MAPPING";
        case EFI_TIMEOUT:
            return (CHAR16 *) L"EFI_TIMEOUT";
        case EFI_NOT_STARTED:
            return (CHAR16 *) L"EFI_NOT_STARTED";
        case EFI_ALREADY_STARTED:
            return (CHAR16 *) L"EFI_ALREADY_STARTED";
        case EFI_ABORTED:
            return (CHAR16 *) L"EFI_ABORTED";
        case EFI_ICMP_ERROR:
            return (CHAR16 *) L"EFI_ICMP_ERROR";
        case EFI_TFTP_ERROR:
            return (CHAR16 *) L"EFI_TFTP_ERROR";
        case EFI_PROTOCOL_ERROR:
            return (CHAR16 *) L"EFI_PROTOCOL_ERROR";
        case EFI_INCOMPATIBLE_VERSION:
            return (CHAR16 *) L"EFI_INCOMPATIBLE_VERSION";
        case EFI_SECURITY_VIOLATION:
            return (CHAR16 *) L"EFI_SECURITY_VIOLATION";
        case EFI_CRC_ERROR:
            return (CHAR16 *) L"EFI_CRC_ERROR";
        case EFI_END_OF_MEDIA:
            return (CHAR16 *) L"EFI_END_OF_MEDIA";
        case EFI_END_OF_FILE:
            return (CHAR16 *) L"EFI_END_OF_FILE";
        case EFI_INVALID_LANGUAGE:
            return (CHAR16 *) L"EFI_INVALID_LANGUAGE";
        case EFI_COMPROMISED_DATA:
            return (CHAR16 *) L"EFI_COMPROMISED_DATA";
        default:
            return (CHAR16 *) L"EFI_UNKNOWN";
    }
}

void throw_error(EFI_SYSTEM_TABLE *system_table, ErrorStatus error) {
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_BACKGROUND_RED | EFI_WHITE);
    system_table->ConOut->OutputString(system_table->ConOut, (CHAR16 *) L" ERR! ");
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_BACKGROUND_BLACK | EFI_LIGHTRED);
    system_table->ConOut->OutputString(system_table->ConOut, (CHAR16 *) L" ");
    system_table->ConOut->OutputString(system_table->ConOut, (CHAR16 *) status_to_str(error));
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_BACKGROUND_BLACK | EFI_LIGHTGRAY);
    system_table->ConOut->OutputString(system_table->ConOut, (CHAR16 *) L"\n\r       EFI Error code: ");
    CHAR16 hex_code[HEX_BUFFER_SIZE];
    to_hex(error.error_code, hex_code);
    system_table->ConOut->OutputString(system_table->ConOut, (CHAR16 *) hex_code);
    system_table->ConOut->OutputString(system_table->ConOut, (CHAR16 *) L" (");
    system_table->ConOut->OutputString(system_table->ConOut, (CHAR16 *) efi_status_to_str(error.error_code));
    system_table->ConOut->OutputString(system_table->ConOut, (CHAR16 *) L")\n\r");
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_BACKGROUND_BLACK | EFI_LIGHTGRAY);
}
