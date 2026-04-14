# KURO Config

Instead of having a separate configuration file, KURO has a configuration at the end of the KURO executable itself.

The configuration is appended to the end of the executable. It is a binary blob that contains the following information:

Total size: 554 bytes.

```c++
typedef struct {
    uint8_t has_public_key;
    uint8_t aslr_enabled;
    uint8_t public_key[32];
    uint16_t executable_path[256];
    KuroIdentifier identifier;
} KuroConfig;
```

#### identifier

```c++
typedef struct {
    char k_magic0;
    char k_magic1;
    char k_magic2;
    char k_magic3;
    char k_magic4;
    uint8_t k_version;
    uint16_t k_reserved;
} KuroIdentifier;
```

#### k_magic

The first five bytes of the KURO identifier are used to identify the executable as a KURO executable. The magic number
is a unique sequence of bytes that is used to identify the executable as a KURO executable. The magic number is `0x7F`
followed by `KURO` in ASCII, which is `0x4B 0x55 0x52 0x4F` in hexadecimal.

The bootloader must verify that the magic number in the KURO identifier matches the expected value before loading the
executable. If the magic number does not match, the bootloader must reject the executable and not load it.

| Byte     | Value  |
|----------|--------|
| k_magic0 | `0x7F` |
| k_magic1 | `0x4B` |
| k_magic2 | `0x55` |
| k_magic3 | `0x52` |
| k_magic4 | `0x4F` |

#### k_version

Any other undefined version number is considered reserved for future use.

| Version | Description                      |
|---------|----------------------------------|
| `0`     | Invalid version (never used)     |
| `1`     | KURO Configuration Version `1.0` |

#### has_public_key

`1` if the KURO has a public key, `0` otherwise.

If KURO has no public key, it will enter insecure mode and will not verify the executable's signature. This is useful
for development purposes but should not be used in production.

Insecure mode is not allowed when secure boot is enabled.

#### aslr_enabled

`1` if ASLR should be enabled, `0` otherwise. If ASLR is enabled, KURO will load the executable at a random address each
time it is booted. This is a security feature that makes it harder for attackers to exploit vulnerabilities in the
executable.

#### public_key

Ed25519 public key of the executable. Used for signature verification. If `has_public_key` is `0`, this field is
ignored.

#### executable_path

A 256-byte array containing the path to the executable which is null-terminated.

The character stored in this array must be UTF-16 encoded.

Path separators are `\` following the UEFI specification, the path is case-insensitive.
