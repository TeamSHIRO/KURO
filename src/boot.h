#ifndef BOOT_H
#define BOOT_H
#include "conf.h"
#include "efi-st.h"
#include "efi.h"
#include "kuro.h"
#include "status.h"

#define PAGE_SIZE 4096

#define STACK_SIZE_PAGE 3

#define STACK_SIZE (STACK_SIZE_PAGE * PAGE_SIZE)

#define TO_PAGES(a) (((a) + 0xfff) / PAGE_SIZE)

typedef struct {
    EFI_MEMORY_DESCRIPTOR *MemoryMap;
    UINTN *MapKey;
    UINTN *DescriptorSize;
    UINT32 *DescriptorVersion;
} MemoryMap;

ErrorStatus boot_elf(EFI_HANDLE image_handle, const EFI_SYSTEM_TABLE *system_table, const KuroConfigInternal *config);

#endif // BOOT_H
