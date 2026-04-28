#include "status.h"
#include <efi.h>

#include "efi_helper.h"
#include "file.h"
#include "string.h"
#include "kuro_conf.h"
#include "protocol/efi-fp.h"

const CHAR16 *status_to_str(ErrorStatus status) {
    enum KuroStatus error = status.status;
    switch (error) {
        case WATCHDOG_DISABLE_FAILED:
            return L"Failed to disable watchdog timer";
        case CONFIG_LOAD_FAILED:
            return L"Failed to load configuration";
        case INVALID_STRING_CONFIG:
            return L"Invalid string table inside the configuration";
        case LOG_INIT_FAILED:
            return L"Failed to initialize file logging";
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
        case SYSTEM_CANNOT_OPEN_PROTOCOL:
            return L"Cannot open or locate a protocol";
        case SYSTEM_CANNOT_GET_VARIABLE:
            return L"Cannot get a system variable";
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

static EFI_FILE_INFO *g_file_info = NULL;
static UINTN g_file_info_size = 80 + 26; // Base size + "kuro_log.txt\0"

static EFI_FILE_PROTOCOL *g_log_file = NULL;

EFI_STATUS init_log_file(EFI_HANDLE image_handle, const EFI_SYSTEM_TABLE *system_table) {
    k_br(system_table, KURO_LOG_LEVEL_INFO);
    k_info(system_table, L"Initializing log file...\r\n");
    EFI_FILE_PROTOCOL *volume;
    EFI_STATUS status = cached_volume_open(image_handle, system_table, &volume);
    if (status != EFI_SUCCESS) {
        return status;
    }

    k_info(system_table, L"Searching for log file...\r\n");
    status = volume->Open(volume, &g_log_file, L"\\kuro_log.txt", EFI_FILE_MODE_READ, 0);
    if (status == EFI_ERR(EFI_NOT_FOUND)) {
        k_info(system_table, L"No log file found! Creating a new one...\r\n");
        status = volume->Open(volume, &g_log_file, L"\\kuro_log.txt", EFI_FILE_MODE_WRITE | EFI_FILE_MODE_READ | EFI_FILE_MODE_CREATE, 0);
        if (status != EFI_SUCCESS) {
            g_log_file->Close(g_log_file);
            g_log_file = NULL;
            return status;
        }
        k_success(system_table, L"Initialized log file!\r\n");
        return EFI_SUCCESS;
    }
    if (status != EFI_SUCCESS) {
        return status;
    }
    k_info(system_table, L"Log file found! Cleaning up logs...\r\n");

    status = system_table->BootServices->AllocatePool(EfiLoaderData, g_file_info_size, (void **) &g_file_info);
    if (status != EFI_SUCCESS) {
        return status;
    }
    status = g_log_file->GetInfo(g_log_file, (EFI_GUID *) &FI_ID, &g_file_info_size, g_file_info);
    if (status != EFI_SUCCESS) {
        system_table->BootServices->FreePool(g_file_info);
        g_log_file->Close(g_log_file);
        g_log_file = NULL;
        return status;
    }

    g_log_file->Delete(g_log_file);
    g_log_file = NULL;

    status = volume->Open(volume, &g_log_file, L"\\kuro_log.txt", EFI_FILE_MODE_WRITE | EFI_FILE_MODE_READ | EFI_FILE_MODE_CREATE, 0);
    if (status != EFI_SUCCESS) {
        system_table->BootServices->FreePool(g_file_info);
        return status;
    }
    k_success(system_table, L"Initialized log file!\r\n");
    return EFI_SUCCESS;
}

void fini_log_file(const EFI_SYSTEM_TABLE *system_table) {
    if (g_log_file == NULL) {
        k_info(system_table, L"No log file to finalize!\r\n");
        return;
    }
    k_info(system_table, L"Finalizing log file...\r\n");

    EFI_TIME g_current_time;
    if (system_table->RuntimeServices->GetTime(&g_current_time, NULL) == EFI_SUCCESS && g_file_info != NULL) {
        g_file_info->LastAccessTime = g_current_time;
        g_file_info->ModificationTime = g_current_time;
        g_log_file->SetInfo(g_log_file, (EFI_GUID *) &FI_ID, g_file_info_size, g_file_info);
    }
    if (g_file_info != NULL) {
        system_table->BootServices->FreePool(g_file_info);
    }

    // Finalize = Setting the time metadata, not closing and flushing :sob:
    k_success(system_table, L"Finalized log file!\r\n");
    g_log_file->Flush(g_log_file);
    g_log_file->Close(g_log_file);
    g_log_file = NULL;
}

static void print(const EFI_SYSTEM_TABLE *system_table, CHAR16 *string) {
    system_table->ConOut->OutputString(system_table->ConOut, string);

    if (g_log_file == NULL) {
        return;
    }
    size_t string_len = wstrlen(string);
    char *string_short;
    EFI_STATUS status = system_table->BootServices->AllocatePool(EfiLoaderData, string_len + 1, (void **) &string_short);
    if (status != EFI_SUCCESS) {
        return;
    }
    to_char(string, string_short, string_len + 1);
    clean_newline(string_short, string_len + 1);
    size_t new_len = strlen(string_short);
    g_log_file->Write(g_log_file, &new_len, string_short);
    system_table->BootServices->FreePool(string_short);
}

void k_error(const EFI_SYSTEM_TABLE *system_table, ErrorStatus error) {
    if (g_console_log_level < KURO_LOG_LEVEL_ERROR) {
        return;
    }
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_RED);
    print(system_table, L"[ERR!]");
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_WHITE);
    print(system_table, L" ");
    print(system_table, (CHAR16 *) status_to_str(error));
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_LIGHTGRAY);
    print(system_table, L"\n\r       EFI Error code: ");
    CHAR16 hex_code[HEX_BUFFER_SIZE];
    to_hex(error.error_code, hex_code);
    print(system_table, hex_code);
    print(system_table, L" (");
    print(system_table, (CHAR16 *) efi_status_to_str(error.error_code));
    print(system_table, L")\n\r");
}

void k_warning(const EFI_SYSTEM_TABLE *system_table, CHAR16 *message) {
    if (g_console_log_level < KURO_LOG_LEVEL_WARNING) {
        return;
    }
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_YELLOW);
    print(system_table, L"[WARN]");
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_WHITE);
    print(system_table, L" ");
    print(system_table, message);
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_LIGHTGRAY);
}

static void k_debug_prefix(const EFI_SYSTEM_TABLE *system_table) {
    print(system_table, L"[DBUG]");
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_WHITE);
    print(system_table, L" ");
}

void k_debug(const EFI_SYSTEM_TABLE *system_table, CHAR16 *message) {
    if (g_console_log_level < KURO_LOG_LEVEL_DEBUG) {
        return;
    }
    k_debug_prefix(system_table);
    print(system_table, message);
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_LIGHTGRAY);
}

void k_debug_hex(const EFI_SYSTEM_TABLE *system_table, CHAR16 *message, uint64_t value) {
    if (g_console_log_level < KURO_LOG_LEVEL_DEBUG) {
        return;
    }
    k_debug_prefix(system_table);
    print(system_table, message);
    CHAR16 hex_code[HEX_BUFFER_SIZE];
    to_hex(value, hex_code);
    print(system_table, L": ");
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_LIGHTGRAY);
    print(system_table, hex_code);
    print(system_table, L"\n\r");

}

void k_debug_num(const EFI_SYSTEM_TABLE *system_table, CHAR16 *message, uint64_t value) {
    if (g_console_log_level < KURO_LOG_LEVEL_DEBUG) {
        return;
    }
    k_debug_prefix(system_table);
    print(system_table, message);
    CHAR16 str_code[STR_BUFFER_SIZE];
    to_str(value, str_code);
    print(system_table, L": ");
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_LIGHTGRAY);
    print(system_table, str_code);
    print(system_table, L"\n\r");
}

void k_debug_str(const EFI_SYSTEM_TABLE *system_table, CHAR16 *message, CHAR16 *info) {
    if (g_console_log_level < KURO_LOG_LEVEL_DEBUG) {
        return;
    }
    k_debug_prefix(system_table);
    print(system_table, message);
    print(system_table, L": ");
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_LIGHTGRAY);
    print(system_table, info);
    print(system_table, L"\n\r");
}

void k_info_str(const EFI_SYSTEM_TABLE *system_table, CHAR16 *message, CHAR16 *info) {
    if (g_console_log_level < KURO_LOG_LEVEL_INFO) {
        return;
    }
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_CYAN);
    print(system_table, L"[INFO]");
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_WHITE);
    print(system_table, L" ");
    print(system_table, message);
    print(system_table, L": ");
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_LIGHTGRAY);
    print(system_table, info);
    print(system_table, L"\n\r");
}

void k_info(const EFI_SYSTEM_TABLE *system_table, CHAR16 *message) {
    if (g_console_log_level < KURO_LOG_LEVEL_INFO) {
        return;
    }
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_CYAN);
    print(system_table, L"[INFO]");
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_WHITE);
    print(system_table, L" ");
    print(system_table, message);
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_LIGHTGRAY);
}

void k_success(const EFI_SYSTEM_TABLE *system_table, CHAR16 *message) {
    if (g_console_log_level < KURO_LOG_LEVEL_INFO) {
        return;
    }
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_GREEN);
    print(system_table, L"[ OK ]");
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_WHITE);
    print(system_table, L" ");
    print(system_table, message);
    system_table->ConOut->SetAttribute(system_table->ConOut, EFI_LIGHTGRAY);
}

void k_br(const EFI_SYSTEM_TABLE *system_table, KuroLogLevel level) {
    if (g_console_log_level < level) {
        return;
    }
    print(system_table, L"\n\r");
}