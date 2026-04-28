#ifndef CONF_H
#define CONF_H

#include <stdint.h>

#include "efi-st.h"
#include "kuro_conf.h"
#include "status.h"

extern const EFI_GUID DPTT_GUID;
extern const EFI_GUID GLOBAL_VARIABLE_GUID;

typedef struct {
    uint8_t secure_mode;
    uint8_t aslr_enabled;
    uint8_t public_key[32];
    KuroLogLevel log_level;
    KuroLogLevel console_log_level;
    char *exec_path;
    char *module_path;
    char *cmd_arg;
    uint8_t free;
} KuroConfigInternal;

ErrorStatus get_config(const EFI_SYSTEM_TABLE *system_table, EFI_HANDLE image_handle, KuroConfigInternal *config);

#endif //CONF_H
