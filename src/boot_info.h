#ifndef BOOT_INFO_H
#define BOOT_INFO_H

#include <stddef.h>

#include "boot.h"
#include "kuro_boot.h"

typedef enum {
    SETUP_DRY_RUN, // only
    SETUP_REAL_RUN
} SetupRunMode;

EFI_STATUS setup_boot_info_and_paging(
    const EFI_SYSTEM_TABLE *system_table,
    KuroSegmentInfo *segments,
    size_t segment_count,
    KuroBootInfo *boot_info,
    char *cmdline, // can be null
    MemoryMap *memory_map,
    KuroModule *module, // can be null
    KuroFramebuffer *framebuffer, // can be null
    KuroExecutableInfo *executable_info, // segment must be null
    SetupRunMode run_mode,
    uint64_t *in_boot_info_base_out_boot_info_size
    // On dry run, will output boot into size in byte, real run will need the base address of boot info, so it can be copied to the right place
    );

#endif //BOOT_INFO_H
