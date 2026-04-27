#include "boot_info.h"

EFI_STATUS setup_boot_info_and_paging(
    const EFI_SYSTEM_TABLE *system_table,
    KuroSegmentInfo *segments,
    size_t segment_count,
    KuroBootInfo *boot_info,
    char *cmdline,
    MemoryMap *memory_map,
    KuroModule *module,
    KuroFramebuffer *framebuffer,
    KuroExecutableInfo *executable_info,
    SetupRunMode run_mode,
    uint64_t *in_boot_info_base_out_boot_info_size) {
    // TODO: Do this
    return EFI_SUCCESS;
}