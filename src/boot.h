#ifndef BOOT_H
#define BOOT_H
#include "efi-st.h"
#include "efi.h"

#define STACK_SIZE_PAGE 3
#define STACK_SIZE (STACK_SIZE_PAGE * 4096)

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
    char pad[4];
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

EFI_STATUS boot_elf(EFI_HANDLE image_handle, const EFI_SYSTEM_TABLE *system_table);

#endif // BOOT_H
