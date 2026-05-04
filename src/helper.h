#ifndef EFI_HELPER_H
#define EFI_HELPER_H

#define EFI_ERR(a) (0x8000000000000000 | (a))
#define ERROR(a, b) (ErrorStatus) {.error_code = (a), .status = (b)}
#define SUCCESS (ErrorStatus) {.error_code = EFI_SUCCESS, .status = K_SUCCESS}

#endif // EFI_HELPER_H
