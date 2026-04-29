#include "verify.h"

#include "ed25519.h"
#include "efi_helper.h"
#include "kuro_const.h"
#include "kuro_footer.h"
#include "string.h"

ErrorStatus verify_footer(unsigned char *file, const size_t FILE_SIZE, const uint8_t public_key[32]) {
    const KuroFooter *footer = (KuroFooter*) (file + FILE_SIZE - sizeof(KuroFooter));

    if (memcmp(footer->k_identifier.k_magic, KURO_MAGIC_CONST, KURO_MAGIC_LEN) != 0 || footer->k_identifier.k_version != KURO_VERSION_1) {
        return (ErrorStatus) {
            .error_code = 0,
            .status = FOOTER_UNREADABLE
        };
    }

    size_t file_no_footer = FILE_SIZE - sizeof(KuroFooter);

    if (ed25519_verify((const unsigned char*) footer->k_signature, file, file_no_footer, public_key) == 0) {
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
