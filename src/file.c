#include "file.h"

#include <stddef.h>

#include "efi.h"
#include "helper.h"
#include "status.h"
#include "protocol/efi-lip.h"
#include "protocol/efi-sfsp.h"

const EFI_GUID LIP_GUID = EFI_LOADED_IMAGE_PROTOCOL_GUID;
const EFI_GUID SFSP_GUID = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
const EFI_GUID FI_ID = EFI_FILE_INFO_ID;

/**
This function caches file protocol in a static variable, so it can be reused without opening the volume again. This
is because we only need to open the volume once, and then we can reuse the file protocol for later file operations.

This optimization is based on the assumption that the volume will not change during the boot process, which is a
reasonable assumption in most cases. If the volume does change, then this caching mechanism may lead to stale file
protocol being used, but in a typical bootloader scenario, this should not be an issue.
Do not close the file protocol unless you made sure that there are no other functions that still use it

May return a dangling pointer on failure
 */
EFI_STATUS cached_volume_open(EFI_HANDLE image_handle, const EFI_SYSTEM_TABLE *system_table, EFI_FILE_PROTOCOL **output) {
    static EFI_LOADED_IMAGE_PROTOCOL *loaded_image = NULL;
    static EFI_FILE_PROTOCOL *file_protocol = NULL;
    static EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *file_system = NULL;

    if (file_protocol != NULL) {
        *output = file_protocol;
        return EFI_SUCCESS;
    }

    EFI_STATUS status = system_table->BootServices->OpenProtocol(image_handle, (EFI_GUID *) &LIP_GUID, (void **) &loaded_image,
                                                 image_handle, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);

    if (status != EFI_SUCCESS) {
        return status;
    }

    status = system_table->BootServices->OpenProtocol(
        loaded_image->DeviceHandle, (EFI_GUID *) &SFSP_GUID, (void **) &file_system, image_handle, NULL,
        EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);

    if (status != EFI_SUCCESS) {
        goto cleanup;
    }

    status = file_system->OpenVolume(file_system, &file_protocol);

    cleanup:
    if (loaded_image != NULL) {
        system_table->BootServices->CloseProtocol(image_handle, (EFI_GUID *) &LIP_GUID, image_handle, NULL);
    }
    if (file_system != NULL) {
        system_table->BootServices->CloseProtocol(loaded_image->DeviceHandle, (EFI_GUID *) &SFSP_GUID, image_handle, NULL);
    }

    *output = file_protocol;
    return status;
}

EFI_STATUS get_file_size(const EFI_SYSTEM_TABLE *system_table, EFI_FILE_PROTOCOL *file_protocol, UINTN *output) {
    EFI_FILE_INFO *file_info = NULL;
    UINTN file_info_size = 0;

    EFI_STATUS status = file_protocol->GetInfo(file_protocol, (EFI_GUID *) &FI_ID,
                                               &file_info_size, NULL);
    if (status != EFI_ERR(EFI_BUFFER_TOO_SMALL)) {
        k_error(system_table, ERROR(status, SYSTEM_CANNOT_GET_FILE_INFO));
        goto fallback;
    }

    status = system_table->BootServices->AllocatePool(EfiLoaderData, file_info_size, (void **) &file_info);
    if (status != EFI_SUCCESS) {
        k_error(system_table, ERROR(status, SYSTEM_OUT_OF_MEMORY));
        goto fallback;
    }
    status = file_protocol->GetInfo(file_protocol, (EFI_GUID *) &FI_ID, &file_info_size,
                                    file_info);
    if (status != EFI_SUCCESS) {
        k_error(system_table, ERROR(status, SYSTEM_CANNOT_GET_FILE_INFO));
        system_table->BootServices->FreePool(file_info);
        goto fallback;
    }

    *output = file_info->FileSize;
    system_table->BootServices->FreePool(file_info);
    return EFI_SUCCESS;

    fallback:
    status = file_protocol->SetPosition(file_protocol, 0xffffffffffffffff);
    if (status != EFI_SUCCESS) {
        return status;
    }
    return file_protocol->GetPosition(file_protocol, output);
}
