#include "boot.h"

#include "efi.h"
#include "efi_helper.h"
#include "elf.h"
#include "file.h"
#include "protocol/efi-fp.h"

EFI_STATUS boot_elf(EFI_HANDLE image_handle, const EFI_SYSTEM_TABLE *system_table) {
    EFI_FILE_PROTOCOL *file_protocol;
    const EFI_STATUS volume_open_status = volume_open(image_handle, system_table, &file_protocol);
    if (volume_open_status != EFI_SUCCESS) {
        return volume_open_status;
    }

    EFI_FILE_PROTOCOL *file;

    EFI_STATUS status = file_protocol->Open(file_protocol, &file, L"kernel", EFI_FILE_MODE_READ, 0);
    if (status != EFI_SUCCESS) {
        goto error;
    }

    Elf64_Ehdr ehdr;
    UINTN ehdr_size = sizeof(ehdr);

    status = file->Read(file, &ehdr_size, &ehdr);
    if (status != EFI_SUCCESS) {
        goto error;
    }

    status = is_valid_elf_header(&ehdr, file);
    if (status == 0) {
        status = EFI_ERR(EFI_LOAD_ERROR);
        system_table->ConOut->OutputString(system_table->ConOut, L"This is NOT a valid ELF file!\r\n");
        goto error;
    }

    system_table->ConOut->OutputString(system_table->ConOut, L"This is a valid ELF file!\r\n");

    status = check_for_rel_section(&ehdr, system_table, file);
    if (status == 0) {
        status = EFI_ERR(EFI_LOAD_ERROR);
        system_table->ConOut->OutputString(system_table->ConOut, L"This ELF file contains relocation section!\r\n");
        goto error;
    }

    system_table->ConOut->OutputString(system_table->ConOut,
                                       L"This ELF file does not contain any relocation section!\r\n");

    file_protocol->Close(file_protocol);
    return EFI_SUCCESS;

error:
    file_protocol->Close(file_protocol);
    return status;
}
