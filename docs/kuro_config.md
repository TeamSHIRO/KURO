# KURO Config

KURO stores its configuration at the end of the executable instead of using a separate configuration file.

The configuration is appended to the executable as a binary blob with the following layout:

**Total size:** 48 bytes
```c++
typedef struct {
    uint8_t has_public_key;
    uint8_t aslr_enabled;
    uint8_t public_key[32];
    char padding[2];
    uint32_t str_offset;
    KuroIdentifier identifier;
} KuroConfig;
```
## Fields

### `has_public_key`

- `1` if KURO has a public key
- `0` otherwise

If KURO has no public key, it enters insecure mode and does not verify the executable signature.  
This is useful for development but should not be used in production.

Insecure mode is not allowed when secure boot is enabled.

### `aslr_enabled`

- `1` to enable ASLR
- `0` to disable ASLR

When enabled, KURO loads the executable at a random physical address each time it boots.

### `public_key`

The Ed25519 public key is used to verify the executable signature.

If `has_public_key` is `0`, this field is ignored.

### `str_offset`

Relative offset to the string table.

`EOF - 48 - str_offset = String Table`

> [!NOTE]
> 48 is the size of the `KuroConfig` struct.

## String Table

The string table appears before the KURO configuration blob and contains the following strings in order:

1. Path to the executable
2. Path to the module
3. Command-line arguments

All strings are null-terminated UTF-8 strings.
Path separators are always `\`.

Example:

All three strings are:
- `\kuro`
- `\mod`
- `arg1 arg2`

| Offset  | `0x00` | `0x01` | `0x02` | `0x03` | `0x04` | `0x05` | `0x06` | `0x07` | `0x08` | `0x09` | `0x0A` | `0x0B` | `0x0C` | `0x0D` | `0x0E` | `0x0F` | `0x10` |
|---------|--------|--------|--------|--------|--------|--------|--------|--------|--------|--------|--------|--------|--------|--------|--------|--------|--------|
| String  | `\`    | `k`    | `u`    | `r`    | `o`    | `\0`   | `\`    | `m`    | `o`    | `d`    | `\0`   | `-`    | `a`    | `r`    | `g`    | `s`    | `\0`   |

> [!IMPORTANT]
> If a string is empty, its place must be occupied by a null terminator (`\0`).

## `KuroIdentifier`

```c++
typedef struct {
    char k_magic0;
    char k_magic1;
    char k_magic2;
    char k_magic3;
    char k_magic4;
    uint8_t k_version;
    char k_reserved[2];
} KuroIdentifier;
```
### `k_magic`

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

### `k_version`

Any undefined version number is reserved for future use.

| Version | Description                      |
|---------|----------------------------------|
| `0`     | Invalid version                  |
| `1`     | KURO Configuration Version `1.0` |
| `2`     | KURO Configuration Version `2.0` |