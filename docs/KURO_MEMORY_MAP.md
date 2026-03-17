[◀ Documentation Home](README.md)

# kuro_memory_map

KURO provides a memory map to the kernel on boot to help with memory-related operations.

```c
struct kuro_memory_map {
  uint64_t descriptors;
  uint64_t size;
  uint32_t descriptor_size;
  uint32_t descriptor_count;
};
```

- **`descriptors`** : A pointer to the array of memory descriptors.
- **`size`** : The size of the memory map in bytes.
- **`descriptor_size`** : The size of each memory descriptor.
- **`descriptor_count`** : The number of memory descriptors.

A simple function could be used to get more details on a specific memory descriptor, as follows:

```c
static const struct kuro_memory_descriptor* memory_descriptor_at(const struct kuro_memory_map* memory_map, uint32_t index) {
  // Get a pointer to the memory descriptors
  const uint8_t* descriptors = (const uint8_t*)(uintptr_t)memory_map->descriptors;

  // Calculate the address of the requested memory descriptor
  return (const struct kuro_memory_descriptor*)(descriptors + ((size_t)index * memory_map->descriptor_size));
}
```

Don't worry, memory descriptors will be explained in the next chapter.

## kuro_memory_descriptor

A memory descriptor is a structure that provides information about a specific region of memory. It includes details such as the base address, length, and type of the memory region.

```c
struct kuro_memory_descriptor {
  uint32_t type;
  uint32_t reserved;
  uint64_t physical_start;
  uint64_t virtual_start;
  uint64_t number_of_pages;
  uint64_t attribute;
};
```

- **`type`**: The type of the memory region.
- **`reserved`**: Reserved for future use.
- **`physical_start`**: The starting physical address of the memory region.
- **`virtual_start`**: The starting virtual address of the memory region.
- **`number_of_pages`**: The number of pages in the memory region.
- **`attribute`**: Attributes of the memory region, such as cacheability and protection flags.

## kuro_memory_type

KURO defines several memory types that can be used in the `type` field of a `kuro_memory_descriptor`. These types indicate the purpose or usage of a memory region.

| Value | Name                                | Description                  |
| ----- | ----------------------------------- | ---------------------------- |
| 0     | `KURO_MEMORY_RESERVED`              | Reserved memory              |
| 1     | `KURO_MEMORY_LOADER_CODE`           | Loader code                  |
| 2     | `KURO_MEMORY_LOADER_DATA`           | Loader data                  |
| 3     | `KURO_MEMORY_BOOT_SERVICES_CODE`    | Boot services code           |
| 4     | `KURO_MEMORY_BOOT_SERVICES_DATA`    | Boot services data           |
| 5     | `KURO_MEMORY_RUNTIME_SERVICES_CODE` | Runtime services code        |
| 6     | `KURO_MEMORY_RUNTIME_SERVICES_DATA` | Runtime services data        |
| 7     | `KURO_MEMORY_CONVENTIONAL`          | Conventional (usable) memory |
| 8     | `KURO_MEMORY_UNUSABLE`              | Unusable memory              |
| 9     | `KURO_MEMORY_ACPI_RECLAIM`          | ACPI reclaimable memory      |
| 10    | `KURO_MEMORY_ACPI_NVS`              | ACPI NVS memory              |
| 11    | `KURO_MEMORY_MMIO`                  | Memory-mapped I/O            |
| 12    | `KURO_MEMORY_MMIO_PORT_SPACE`       | MMIO port space              |
| 13    | `KURO_MEMORY_PAL_CODE`              | PAL code                     |
| 14    | `KURO_MEMORY_PERSISTENT`            | Persistent memory            |
