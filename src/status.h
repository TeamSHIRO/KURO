#ifndef ERROR_H
#define ERROR_H

#include "efi.h"
#include "kuro_conf.h"

#define INITIAL_STR_HEAP 64

typedef enum {
    WATCHDOG_DISABLE_FAILED,

    CONFIG_LOAD_FAILED,
    INVALID_STRING_CONFIG,

    LOG_INIT_FAILED,
    LOG_WRITE_FAILED,
    INIT_STR_HEAP_FAILED,

    FILE_UNREADABLE,
    FILE_FAILED_SET_POS,

    ELF_INVALID_FILE_SIZE,
    ELF_INVALID_HEADER,
    ELF_UNSUPPORTED_ARCH,
    ELF_UNSUPPORTED_ENDIAN,
    ELF_UNSUPPORTED_VERSION,
    ELF_NOT_64_BIT,
    ELF_NOT_DYN,
    ELF_INVALID_PROGRAM_HEADER,
    ELF_INVALID_MEMORY_SIZE,
    ELF_OUT_OF_BOUND,
    ELF_INVALID_DYN_SECTION,
    ELF_INVALID_ENTRY,
    ELF_INVALID_RELOCATION,
    ELF_INVALID_SYMBOL,

    LOAD_FAILED,
    FOOTER_UNREADABLE,
    SIGNATURE_INVALID,

    FRAMEBUFFER_MODE_FETCH_FAILED,
    FRAMEBUFFER_PREPARE_FAILED,

    SYSTEM_OUT_OF_MEMORY,
    SYSTEM_EXIT_BOOT_SERVICES_FAILED,
    SYSTEM_CANNOT_OPEN_VOLUME,
    SYSTEM_CANNOT_READ_FILESIZE,
    SYSTEM_CANNOT_OPEN_PROTOCOL,
    SYSTEM_CANNOT_GET_VARIABLE,
    SYSTEM_CANNOT_GET_FILE,
    SYSTEM_CANNOT_GET_FILE_INFO,
    SYSTEM_CANNOT_SET_FILE_INFO,
    SYSTEM_CANNOT_GET_TIME,
    SYSTEM_CANNOT_FLUSH_FILE,

    K_UNKNOWN,
    K_SUCCESS
} KuroStatus;

typedef struct {
    EFI_STATUS error_code; // This one tells us what error code is it exactly
    KuroStatus status;
} ErrorStatus;

const CHAR16 *status_to_str(ErrorStatus status);

extern KuroLogLevel g_console_log_level;
extern KuroLogLevel g_file_log_level;

ErrorStatus init_log_file(EFI_HANDLE image_handle, const EFI_SYSTEM_TABLE *system_table);
void fini_log_file(const EFI_SYSTEM_TABLE *system_table);

void k_error(const EFI_SYSTEM_TABLE *system_table, ErrorStatus error);
void k_warning(const EFI_SYSTEM_TABLE *system_table, CHAR16 *message);
void k_debug(const EFI_SYSTEM_TABLE *system_table, CHAR16 *message);
void k_debug_hex(const EFI_SYSTEM_TABLE *system_table, CHAR16 *message, uint64_t value);
void k_debug_num(const EFI_SYSTEM_TABLE *system_table, CHAR16 *message, uint64_t value);
void k_debug_str(const EFI_SYSTEM_TABLE *system_table, CHAR16 *message, CHAR16 *info);
void k_info_str(const EFI_SYSTEM_TABLE *system_table, CHAR16 *message, CHAR16 *info);
void k_info(const EFI_SYSTEM_TABLE *system_table, CHAR16 *message);
void k_success(const EFI_SYSTEM_TABLE *system_table, CHAR16 *message);
void k_br(const EFI_SYSTEM_TABLE *system_table, KuroLogLevel level);

#endif // ERROR_H
