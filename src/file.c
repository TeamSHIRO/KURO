#include "file.h"

#include <stddef.h>

#include "efi.h"
#include "efi_helper.h"
#include "protocol/efi-lip.h"
#include "protocol/efi-sfsp.h"

const EFI_GUID LIP_GUID = EFI_LOADED_IMAGE_PROTOCOL_GUID;
const EFI_GUID SFSP_GUID = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
const EFI_GUID FI_ID = EFI_FILE_INFO_ID;

EFI_STATUS volume_open(EFI_HANDLE image_handle, const EFI_SYSTEM_TABLE *system_table, EFI_FILE_PROTOCOL **output) {
    EFI_LOADED_IMAGE_PROTOCOL *loaded_image;

    const EFI_STATUS GET_LIP_SUCCESS =
        system_table->BootServices->OpenProtocol(image_handle, (EFI_GUID *) &LIP_GUID, (void **) &loaded_image,
                                                 image_handle, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);

    if (GET_LIP_SUCCESS != EFI_SUCCESS) {
        return GET_LIP_SUCCESS;
    }

    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *file_system;

    const EFI_STATUS GET_SFSP_SUCCESS = system_table->BootServices->OpenProtocol(
        loaded_image->DeviceHandle, (EFI_GUID *) &SFSP_GUID, (void **) &file_system, image_handle, NULL,
        EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);

    if (GET_SFSP_SUCCESS != EFI_SUCCESS) {
        system_table->BootServices->CloseProtocol(loaded_image->DeviceHandle, (EFI_GUID *) &SFSP_GUID, image_handle,
                                                  NULL);
        return GET_SFSP_SUCCESS;
    }

    EFI_FILE_PROTOCOL *file_protocol;
    const EFI_STATUS OPEN_VOLUME_SUCCESS = file_system->OpenVolume(file_system, &file_protocol);

    if (OPEN_VOLUME_SUCCESS != EFI_SUCCESS) {
        system_table->BootServices->CloseProtocol(loaded_image->DeviceHandle, (EFI_GUID *) &SFSP_GUID, image_handle,
                                                  NULL);
        return OPEN_VOLUME_SUCCESS;
    }

    system_table->BootServices->CloseProtocol(loaded_image->DeviceHandle, (EFI_GUID *) &SFSP_GUID, image_handle, NULL);
    system_table->BootServices->CloseProtocol(image_handle, (EFI_GUID *) &LIP_GUID, image_handle, NULL);

    *output = file_protocol;

    return EFI_SUCCESS;
}

EFI_STATUS get_file_size(const EFI_SYSTEM_TABLE *system_table, const EFI_FILE_PROTOCOL *file_protocol, UINTN *output) {
    EFI_FILE_INFO *file_info = 0;
    UINTN file_info_size = 0;

    EFI_STATUS status = file_protocol->GetInfo((EFI_FILE_PROTOCOL *) file_protocol, (EFI_GUID *) &FI_ID,
                                               &file_info_size, file_info);
    if (status != EFI_ERR(EFI_BUFFER_TOO_SMALL)) {
        return status;
    }

    if (file_info_size <= sizeof(EFI_FILE_INFO)) {
        file_info_size = sizeof(EFI_FILE_INFO);
        status = file_protocol->GetInfo((EFI_FILE_PROTOCOL *) file_protocol, (EFI_GUID *) &FI_ID, &file_info_size,
                                        file_info);
        if (status != EFI_SUCCESS) {
            return status;
        }
        goto finalize;
    }

    status = system_table->BootServices->AllocatePool(EfiLoaderData, file_info_size, (void **) &file_info);
    if (status != EFI_SUCCESS) {
        return status;
    }

    status = file_protocol->GetInfo((EFI_FILE_PROTOCOL *) file_protocol, (EFI_GUID *) &FI_ID, &file_info_size,
                                    file_info);
    if (status != EFI_SUCCESS) {
        system_table->BootServices->FreePool(file_info);
        return status;
    }

finalize:
    *output = file_info->FileSize;
    system_table->BootServices->FreePool(file_info);
    return EFI_SUCCESS;
}
