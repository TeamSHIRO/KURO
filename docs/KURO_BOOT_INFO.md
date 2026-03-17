[◀ Documentation Home](README.md)

# kuro_boot_info

KURO is passing to the main entry point of the kernel a `kuro_boot_info` pointer with all of the APIs and data needed for the OS (of course, only at an early stage), such as memory layout, hardware information, and a simple framebuffer.

```c
struct kuro_boot_info {
  uint64_t magic;
  uint32_t version;
  uint32_t size;
  struct kuro_framebuffer framebuffer;
  struct kuro_memory_map memory_map;
};
```

- **`magic`** : A magic number used to verify the integrity of the boot information (always `0x4B55524F` or `KURO` in ASCII).
- **`version`** : The version of the boot information structure (currently 1).
- **`size`** : The size of the boot information structure.
- **`framebuffer`** : Information about the framebuffer.
- **`memory_map`** : Information about the system's memory layout.
  Using those APIs, the kernel can then use them to interact with the system, as shown in the following example

```c
#include "boot_info.h" // Header file declaring the `kuro_boot_info` structure

int _start(const struct kuro_boot_info* boot_info) {
  // Access the framebuffer
  const struct kuro_framebuffer* fb = &boot_info->framebuffer;

  // Access the memory map
  const struct kuro_memory_map* memory_map = &boot_info->memory_map;

  // (...)

  return 42;
  // ^^^ Don't forget to return a valid response!
}
```

Learn more about the [Framebuffer](KURO_FRAMEBUFFER.md) and [Memory Map](KURO_MEMORY_MAP.md).
