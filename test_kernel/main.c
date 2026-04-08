#include "main.h"
#include <efi.h>
#include <stddef.h>

typedef struct {
    char k_magic0;
    char k_magic1;
    char k_magic2;
    char k_magic3;
    char k_magic4;
    uint8_t k_version;
    uint16_t k_reserved;
} KuroIdentifier;

typedef struct {
    uint32_t ks_flags;
    char pad[4]; // padding to align the structure to 8 bytes
    uint64_t ks_address;
    uint64_t ks_size;
    uint64_t ks_align;
} KuroSegmentInfo;

typedef struct {
    KuroIdentifier ke_identifier;
    uint64_t ke_entry_point;
    uint64_t ke_segment_count;
    KuroSegmentInfo *ke_segments;
    uint64_t ke_stack_start;
    uint64_t ke_stack_end;
    uint64_t ke_stack_size;
} KuroExecutableInfo;

void to_wchar(const char *src, CHAR16 *dest, size_t max_len) {
    size_t i = 0;
    while (src[i] != '\0' && i < max_len - 1) {
        dest[i] = (CHAR16) src[i];
        i++;
    }
    dest[i] = L'\0';
}

_Noreturn void _start(KuroExecutableInfo *exec_info, EFI_HANDLE image_handle, const EFI_SYSTEM_TABLE *system_table,
                      void *data, const char *boot_id) {
    system_table->ConOut->OutputString(system_table->ConOut, L"Hello, ");
    CHAR16 boot_id_wchar[16];
    to_wchar(boot_id, boot_id_wchar, 16);
    system_table->ConOut->OutputString(system_table->ConOut, boot_id_wchar);
    system_table->ConOut->OutputString(system_table->ConOut, L"!\r\n");
    while (1) {
    }
}
