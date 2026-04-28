#include "verify.h"

#include "ed25519.h"
#include "efi_helper.h"
#include "kuro_const.h"
#include "kuro_footer.h"
#include "string.h"

ErrorStatus verify_footer(EFI_FILE_PROTOCOL *file, const size_t FILE_SIZE, const uint8_t public_key[32]) {
    EFI_STATUS status = file->SetPosition(file, FILE_SIZE - sizeof(KuroFooter));
    if (status != EFI_SUCCESS) {
        return (ErrorStatus) {
            .error_code = status,
            .status = FOOTER_UNREADABLE
        };
    }
    KuroFooter footer;
    size_t footer_size = sizeof(KuroFooter);
    status = file->Read(file, &footer_size, &footer);
    if (status != EFI_SUCCESS) {
        return (ErrorStatus) {
            .error_code = status,
            .status = FOOTER_UNREADABLE
        };
    }

    if (memcmp(footer.k_identifier.k_magic, KURO_MAGIC_CONST, KURO_MAGIC_LEN) != 0 || footer.k_identifier.k_version != KURO_VERSION_1) {
        return (ErrorStatus) {
            .error_code = EFI_ERR(EFI_INVALID_PARAMETER),
            .status = FOOTER_UNREADABLE
        };
    }

    size_t file_no_footer = FILE_SIZE - footer_size;
    unsigned char *file_char;

    status = file->SetPosition(file, 0);
    if (status != EFI_SUCCESS) {
        return (ErrorStatus) {
            .error_code = status,
            .status = FOOTER_UNREADABLE
        };
    }

    status = file->Read(file, &file_no_footer, &file_char);
    if (status != EFI_SUCCESS) {
        return (ErrorStatus) {
            .error_code = status,
            .status = FOOTER_UNREADABLE
        };
    }

    if (ed25519_verify((const unsigned char*) footer.k_signature, file_char, file_no_footer, public_key) == 0) {
        return (ErrorStatus) {
            .error_code = EFI_ERR(EFI_LOAD_ERROR),
            .status = SIGNATURE_INVALID
        };
    }
    return (ErrorStatus) {
        .error_code = EFI_SUCCESS,
        .status = SUCCESS
    };
}
