[◀ Documentation Home](README.md)

# kuro_framebuffer

KURO's framebuffer is a simple structure that provides information about the display, such as its base address, width, height, and pixel format. This allows the kernel to draw directly to the screen. It is similar to multiboot's VGA output framebuffer.

```c
struct kuro_framebuffer {
  uint64_t base;
  uint64_t size;
  uint32_t width;
  uint32_t height;
  uint32_t pixels_per_scanline;
  uint32_t pixel_format;
  uint32_t red_mask;
  uint32_t green_mask;
  uint32_t blue_mask;
  uint32_t reserved_mask;
};
```

- **`base`** : The base address of the framebuffer.
- **`size`** : The size of the framebuffer in bytes.
- **`width`** : The width of the framebuffer in pixels.
- **`height`** : The height of the framebuffer in pixels.
- **`pixels_per_scanline`** : The number of pixels per scanline.
- **`pixel_format`** : The format of the pixels.
- **`red_mask`** : The mask for the red component of a pixel.
- **`green_mask`** : The mask for the green component of a pixel.
- **`blue_mask`** : The mask for the blue component of a pixel.
- **`reserved_mask`** : The mask for the reserved component of a pixel.

Example function to draw a pixel to the screen using the `kuro_framebuffer` api:

```c
static void put_pixel(const struct kuro_framebuffer* fb, uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b) {
  // Prevent overflow out of the display
  if (x >= fb->width || y >= fb->height) return;

  // Get a pointer to the framebuffer's pixel data
  volatile uint32_t* pixels = (volatile uint32_t*)(uintptr_t)fb->base;

  // Calculate the offset of the pixel in the framebuffer
  const uint64_t offset =
      (uint64_t)y * (uint64_t)fb->pixels_per_scanline + (uint64_t)x;
  pixels[offset] = encode_pixel(fb, r, g, b);
}
```
