# KURO Config

Instead of having a separate configuration file, KURO has a configuration at the end of the KURO executable itself.

The configuration is appended to the end of the executable. It is a binary blob that contains the following information:

- Magic number: A fixed value that identifies the configuration as a KURO configuration.
- Version: The version of the configuration format.
- Executable path: The path to the executable.
- Public key present: Whether KURO has a public key or not, If not, KURO will enter unsecure mode and will not verify
  the executable's signature.
- Public key: The public key of the executable, used for signature verification.
- ASLR: Whether should KURO load the executable with ASLR enabled.

Total size: 554 bytes.

```c++
struct KuroConfig {
    KuroIdentifier identifier;
    uint8_t has_public_key;
    uint8_t aslr_enabled;
    uint8_t public_key[32];
    uint16_t executable_path[256];
};
```

#### identifier

As described in [section 4.1](kuro_booting_convention.md#41-kuro-identifier) of the KURO booting convention.

#### has_public_key

`1` if the KURO has a public key, `0` otherwise.

If KURO has no public key, it will enter unsecure mode and will not verify the executable's signature. This is useful
for development purposes but should not be used in production.

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
