# KURO Config

KURO stores its configuration at the end of the executable instead of using a separate configuration file.

By default, if KURO cannot find the configuration, KURO will print an error message to the console and return.

The configuration is appended to the executable as a binary blob with the following layout:

**Total size:** 48 bytes

```c++
typedef struct {
    uint8_t secure_mode;
    uint8_t aslr_enabled;
    uint8_t public_key[32];
    uint8_t log_level;
    uint8_t console_log_level;
    uint32_t str_offset;
    KuroConfigIdentifier identifier;
} KuroConfig;
```

#### secure_mode

- `1` true
- `0` false

If false, it enters insecure mode and does not verify the executable signature.  
This is useful for development but should not be used in production.

Insecure mode is not allowed when secure boot is enabled.

#### aslr_enabled

- `1` to enable ASLR
- `0` to disable ASLR

When enabled, KURO loads the executable at a random physical address each time it boots.

#### public_key

The Ed25519 public key is used to verify the executable signature.

If `secure_mode` is `0`, this field is ignored.

#### log_level

- `0` for disabled logging
- `1` for ERROR level
- `2` for WARNING level
- `3` for INFO level
- `4` for DEBUG level

When logging is enabled, only messages with a level less than or equal to the configured log level are logged.

If logging is disabled, KURO will not save any log file (kuro_log.txt) to the root directory of the ESP partition where
it is located.

#### console_log_level

Same as `log_level` but affects the console output (UEFI ConOut) instead of the log file.

If disabled, KURO will clear the screen once at the early part of the logic and not print any messages to the console.

#### str_offset

Relative offset to the string table.

$ EOF - sizeof(KuroConfig) - str\_offset = String Table $

## String Table

The string table appears before the KURO configuration blob and contains the following strings in order:

1. Path to the executable
2. Path to the module
3. Command-line arguments

All strings are null-terminated ASCII strings except for the command-line arguments.
Path separators are always `\`.

Example:

All three strings are:
- `\kuro`
- `\mod`
- `-args`

| Offset  | `0x00` | `0x01` | `0x02` | `0x03` | `0x04` | `0x05` | `0x06` | `0x07` | `0x08` | `0x09` | `0x0A` | `0x0B` | `0x0C` | `0x0D` | `0x0E` | `0x0F` | `0x10` |
|---------|--------|--------|--------|--------|--------|--------|--------|--------|--------|--------|--------|--------|--------|--------|--------|--------|--------|
| String  | `\`    | `k`    | `u`    | `r`    | `o`    | `\0`   | `\`    | `m`    | `o`    | `d`    | `\0`   | `-`    | `a`    | `r`    | `g`    | `s`    | `\0`   |

> [!IMPORTANT]
> If a string is empty, its place must be occupied by a null terminator (`\0`).
> All path must start with `\` and cannot end with `\`.

> [!CAUTION]
> Path to the executable must not be empty!

## KuroConfigIdentifier

```c++
#define KURO_MAGIC {0x7F, 0x4B, 0x55, 0x52, 0x4F}
#define KURO_MAGIC_LEN 5

typedef struct {
    char k_magic[KURO_MAGIC_LEN];
    uint8_t k_version;
    char k_reserved[2];
} KuroConfigIdentifier;
```
#### k_magic

The magic value is a fixed sequence of bytes:

- `0x7F`
- `KURO` in ASCII, which corresponds to:
  - `0x4B`
  - `0x55`
  - `0x52`
  - `0x4F`

| Byte     | Value  |
|----------|--------|
| k_magic0 | `0x7F` |
| k_magic1 | `0x4B` |
| k_magic2 | `0x55` |
| k_magic3 | `0x52` |
| k_magic4 | `0x4F` |

#### k_version

Any undefined version number is reserved for future use.

| Version | Description                      |
|---------|----------------------------------|
| `0`     | Invalid version                  |
| `1`     | KURO Configuration Version `1.0` |