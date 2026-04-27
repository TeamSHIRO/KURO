#include "status.h"
#include <efi.h>
#include "string.h"
#include "kuro_conf.h"

const CHAR16 *status_to_str(ErrorStatus status) {
    enum KuroStatus error = status.status;
    switch (error) {
        case WATCHDOG_DISABLE_FAILED:
            return L"Failed to disable watchdog timer";
        case ELF_FILE_NOT_FOUND:
            return L"ELF file not found";
        case ELF_UNREADABLE:
            return L"ELF file is unreadable";
        case ELF_FAILED_SET_POS:
            return L"Failed to set position in ELF file";
        case ELF_INVALID_FILE_SIZE:
            return L"ELF file has an impossible file size";
        case ELF_INVALID_HEADER:
            return L"ELF file has an invalid ELF header";
        case ELF_INVALID_PROGRAM_HEADER:
            return L"ELF file has an invalid program header";
        case ELF_INVALID_MEMORY_SIZE:
            return L"ELF file has an invalid memory size";
        case LOAD_FAILED:
            return L"Failed to load the executable";
        case FRAMEBUFFER_MODE_FETCH_FAILED:
            return L"Failed to fetch framebuffer mode";
        case FRAMEBUFFER_PREPARE_FAILED:
            return L"Failed to prepare framebuffer";
        case SYSTEM_OUT_OF_MEMORY:
            return L"System is out of memory";
        case SYSTEM_ALLOCATION_FAILED:
            return L"System memory allocation failed";
        case SYSTEM_EXIT_BOOT_SERVICES_FAILED:
            return L"Failed to exit boot services";
        case SYSTEM_CANNOT_OPEN_VOLUME:
            return L"Cannot open volume";
        case SYSTEM_CANNOT_READ_FILESIZE:
            return L"Cannot read filesize";
        case SUCCESS:
            return L"Success";
        case UNKNOWN:
        default:
            return L"An unexpected error occurred";
    }
}

const CHAR16 *efi_status_to_str(EFI_STATUS status) {
    EFI_STATUS status_code = status & 0x7FFFFFFFFFFFFFFF;
    switch (status_code) {
        case EFI_SUCCESS:
            return L"EFI_SUCCESS";
        case EFI_LOAD_ERROR:
            return L"EFI_LOAD_ERROR";
        case EFI_INVALID_PARAMETER:
            return L"EFI_INVALID_PARAMETER";
        case EFI_UNSUPPORTED:
            return L"EFI_UNSUPPORTED";
        case EFI_BAD_BUFFER_SIZE:
            return L"EFI_BAD_BUFFER_SIZE";
        case EFI_BUFFER_TOO_SMALL:
            return L"EFI_BUFFER_TOO_SMALL";
        case EFI_NOT_READY:
            return L"EFI_NOT_READY";
        case EFI_DEVICE_ERROR:
            return L"EFI_DEVICE_ERROR";
        case EFI_WRITE_PROTECTED:
            return L"EFI_WRITE_PROTECTED";
        case EFI_OUT_OF_RESOURCES:
            return L"EFI_OUT_OF_RESOURCES";
        case EFI_VOLUME_CORRUPTED:
            return L"EFI_VOLUME_CORRUPTED";
        case EFI_VOLUME_FULL:
            return L"EFI_VOLUME_FULL";
        case EFI_NO_MEDIA:
            return L"EFI_NO_MEDIA";
        case EFI_MEDIA_CHANGED:
            return L"EFI_MEDIA_CHANGED";
        case EFI_NOT_FOUND:
            return L"EFI_NOT_FOUND";
        case EFI_ACCESS_DENIED:
            return L"EFI_ACCESS_DENIED";
        case EFI_NO_RESPONSE:
            return L"EFI_NO_RESPONSE";
        case EFI_NO_MAPPING:
            return L"EFI_NO_MAPPING";
        case EFI_TIMEOUT:
            return L"EFI_TIMEOUT";
        case EFI_NOT_STARTED:
            return L"EFI_NOT_STARTED";
        case EFI_ALREADY_STARTED:
            return L"EFI_ALREADY_STARTED";
        case EFI_ABORTED:
            return L"EFI_ABORTED";
        case EFI_ICMP_ERROR:
            return L"EFI_ICMP_ERROR";
        case EFI_TFTP_ERROR:
            return L"EFI_TFTP_ERROR";
        case EFI_PROTOCOL_ERROR:
            return L"EFI_PROTOCOL_ERROR";
        case EFI_INCOMPATIBLE_VERSION:
            return L"EFI_INCOMPATIBLE_VERSION";
        case EFI_SECURITY_VIOLATION:
            return L"EFI_SECURITY_VIOLATION";
        case EFI_CRC_ERROR:
            return L"EFI_CRC_ERROR";
        case EFI_END_OF_MEDIA:
            return L"EFI_END_OF_MEDIA";
        case EFI_END_OF_FILE:
            return L"EFI_END_OF_FILE";
        case EFI_INVALID_LANGUAGE:
            return L"EFI_INVALID_LANGUAGE";
        case EFI_COMPROMISED_DATA:
            return L"EFI_COMPROMISED_DATA";
        default:
            return L"EFI_UNKNOWN";
    }
}

// Bad idea, but who's seriously going to change this except the conf?
KuroLogLevel g_console_log_level = KURO_LOG_LEVEL_DEBUG;

void k_error(const EFI_SYSTEM_TABLE *system_table, ErrorStatus error) {
    if (g_console_log_level < KURO_LOG_LEVEL_ERROR) {
        return;
    }
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_BACKGROUND_RED | EFI_WHITE);
    system_table->ConOut->OutputString(system_table->ConOut, L" ERR! ");
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_BACKGROUND_BLACK | EFI_WHITE);
    system_table->ConOut->OutputString(system_table->ConOut, L" ");
    system_table->ConOut->OutputString(system_table->ConOut, (CHAR16 *) status_to_str(error));
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_BACKGROUND_BLACK | EFI_LIGHTGRAY);
    system_table->ConOut->OutputString(system_table->ConOut, L"\n\r       EFI Error code: ");
    CHAR16 hex_code[HEX_BUFFER_SIZE];
    to_hex(error.error_code, hex_code);
    system_table->ConOut->OutputString(system_table->ConOut, hex_code);
    system_table->ConOut->OutputString(system_table->ConOut, L" (");
    system_table->ConOut->OutputString(system_table->ConOut, (CHAR16 *) efi_status_to_str(error.error_code));
    system_table->ConOut->OutputString(system_table->ConOut, L")\n\r");
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_BACKGROUND_BLACK | EFI_LIGHTGRAY);
}

void k_warning(const EFI_SYSTEM_TABLE *system_table, CHAR16 *message) {
    if (g_console_log_level < KURO_LOG_LEVEL_WARNING) {
        return;
    }
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_BACKGROUND_RED | EFI_YELLOW);
    system_table->ConOut->OutputString(system_table->ConOut, L" WARN ");
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_BACKGROUND_BLACK | EFI_WHITE);
    system_table->ConOut->OutputString(system_table->ConOut, L" ");
    system_table->ConOut->OutputString(system_table->ConOut, message);
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_BACKGROUND_BLACK | EFI_LIGHTGRAY);
}

static void k_debug_prefix(const EFI_SYSTEM_TABLE *system_table) {
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_BACKGROUND_LIGHTGRAY | EFI_BLACK);
    system_table->ConOut->OutputString(system_table->ConOut, L" DBUG ");
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_BACKGROUND_BLACK | EFI_WHITE);
    system_table->ConOut->OutputString(system_table->ConOut, L" ");
}

void k_debug(const EFI_SYSTEM_TABLE *system_table, CHAR16 *message) {
    if (g_console_log_level < KURO_LOG_LEVEL_DEBUG) {
        return;
    }
    k_debug_prefix(system_table);
    system_table->ConOut->OutputString(system_table->ConOut, message);
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_BACKGROUND_BLACK | EFI_LIGHTGRAY);
}

void k_debug_hex(const EFI_SYSTEM_TABLE *system_table, CHAR16 *message, uint64_t value) {
    if (g_console_log_level < KURO_LOG_LEVEL_DEBUG) {
        return;
    }
    k_debug_prefix(system_table);
    system_table->ConOut->OutputString(system_table->ConOut, message);
    CHAR16 hex_code[HEX_BUFFER_SIZE];
    to_hex(value, hex_code);
    system_table->ConOut->OutputString(system_table->ConOut, L": ");
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_BACKGROUND_BLACK | EFI_LIGHTGRAY);
    system_table->ConOut->OutputString(system_table->ConOut, hex_code);
    system_table->ConOut->OutputString(system_table->ConOut, L"\n\r");

}

void k_debug_num(const EFI_SYSTEM_TABLE *system_table, CHAR16 *message, uint64_t value) {
    if (g_console_log_level < KURO_LOG_LEVEL_DEBUG) {
        return;
    }
    k_debug_prefix(system_table);
    system_table->ConOut->OutputString(system_table->ConOut, message);
    CHAR16 str_code[STR_BUFFER_SIZE];
    to_str(value, str_code);
    system_table->ConOut->OutputString(system_table->ConOut, L": ");
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_BACKGROUND_BLACK | EFI_LIGHTGRAY);
    system_table->ConOut->OutputString(system_table->ConOut, str_code);
    system_table->ConOut->OutputString(system_table->ConOut, L"\n\r");
}

void k_debug_str(const EFI_SYSTEM_TABLE *system_table, CHAR16 *message, CHAR16 *info) {
    if (g_console_log_level < KURO_LOG_LEVEL_DEBUG) {
        return;
    }
    k_debug_prefix(system_table);
    system_table->ConOut->OutputString(system_table->ConOut, message);
    system_table->ConOut->OutputString(system_table->ConOut, L": ");
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_BACKGROUND_BLACK | EFI_LIGHTGRAY);
    system_table->ConOut->OutputString(system_table->ConOut, info);
    system_table->ConOut->OutputString(system_table->ConOut, L"\n\r");
}

void k_info(const EFI_SYSTEM_TABLE *system_table, CHAR16 *message) {
    if (g_console_log_level < KURO_LOG_LEVEL_INFO) {
        return;
    }
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_BACKGROUND_BLUE | EFI_WHITE);
    system_table->ConOut->OutputString(system_table->ConOut, L" INFO ");
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_BACKGROUND_BLACK | EFI_WHITE);
    system_table->ConOut->OutputString(system_table->ConOut, L" ");
    system_table->ConOut->OutputString(system_table->ConOut, message);
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_BACKGROUND_BLACK | EFI_LIGHTGRAY);
}

void k_success(const EFI_SYSTEM_TABLE *system_table, CHAR16 *message) {
    if (g_console_log_level < KURO_LOG_LEVEL_INFO) {
        return;
    }
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_BACKGROUND_GREEN | EFI_WHITE);
    system_table->ConOut->OutputString(system_table->ConOut, L"  OK  ");
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_BACKGROUND_BLACK | EFI_WHITE);
    system_table->ConOut->OutputString(system_table->ConOut, L" ");
    system_table->ConOut->OutputString(system_table->ConOut, message);
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_BACKGROUND_BLACK | EFI_LIGHTGRAY);
}
