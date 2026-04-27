#ifndef CONF_H
#define CONF_H

#include <stdint.h>

#include "efi-st.h"
#include "kuro_conf.h"

typedef struct {
    uint8_t secure_mode;
    uint8_t aslr_enabled;
    uint8_t public_key[32];
    KuroLogLevel log_level;
    KuroLogLevel console_log_level;
    char *exec_path;
    char *module_path;
    char *cmd_arg;
} KuroConfigInternal;

void get_config(const EFI_SYSTEM_TABLE *system_table, EFI_HANDLE image_handle, KuroConfigInternal *config);

#endif //CONF_H
