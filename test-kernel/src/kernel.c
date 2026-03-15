/*
 * File: kernel.c
 * Description: Minimal monolithic 64-bit kernel entry for KURO.
 *
 * Copyright (C) 2026 Ellicode
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stddef.h>
#include <stdint.h>

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

struct kuro_boot_info {
  struct kuro_framebuffer framebuffer;
};

enum {
  KURO_PIXEL_RGBR_8 = 0,
  KURO_PIXEL_BGRR_8 = 1,
  KURO_PIXEL_BITMASK = 2,
};

static uint32_t lsb_index(uint32_t mask) {
  uint32_t idx = 0;
  if (mask == 0) return 0;
  while (((mask >> idx) & 1U) == 0U && idx < 31U) {
    idx++;
  }
  return idx;
}

static uint32_t encode_pixel(const struct kuro_framebuffer* fb, uint8_t r,
                             uint8_t g, uint8_t b) {
  if (fb->pixel_format == KURO_PIXEL_RGBR_8) {
    return ((uint32_t)b) | ((uint32_t)g << 8) | ((uint32_t)r << 16);
  }

  if (fb->pixel_format == KURO_PIXEL_BGRR_8) {
    return ((uint32_t)r) | ((uint32_t)g << 8) | ((uint32_t)b << 16);
  }

  if (fb->pixel_format == KURO_PIXEL_BITMASK) {
    uint32_t out = 0;
    out |= ((uint32_t)r << lsb_index(fb->red_mask)) & fb->red_mask;
    out |= ((uint32_t)g << lsb_index(fb->green_mask)) & fb->green_mask;
    out |= ((uint32_t)b << lsb_index(fb->blue_mask)) & fb->blue_mask;
    return out;
  }

  return ((uint32_t)b) | ((uint32_t)g << 8) | ((uint32_t)r << 16);
}

static void put_pixel(const struct kuro_framebuffer* fb, uint32_t x, uint32_t y,
                      uint8_t r, uint8_t g, uint8_t b) {
  if (x >= fb->width || y >= fb->height) return;
  volatile uint32_t* pixels = (volatile uint32_t*)(uintptr_t)fb->base;
  const uint64_t offset =
      (uint64_t)y * (uint64_t)fb->pixels_per_scanline + (uint64_t)x;
  pixels[offset] = encode_pixel(fb, r, g, b);
}

static void fill_rect(const struct kuro_framebuffer* fb, uint32_t x, uint32_t y,
                      uint32_t w, uint32_t h, uint8_t r, uint8_t g, uint8_t b) {
  for (uint32_t yy = 0; yy < h; ++yy) {
    for (uint32_t xx = 0; xx < w; ++xx) {
      put_pixel(fb, x + xx, y + yy, r, g, b);
    }
  }
}

static const uint8_t* glyph_for(char c) {
  static const uint8_t H[8] = {0x42, 0x42, 0x42, 0x7E, 0x42, 0x42, 0x42, 0x00};
  static const uint8_t E[8] = {0x7E, 0x40, 0x40, 0x7C, 0x40, 0x40, 0x7E, 0x00};
  static const uint8_t L[8] = {0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x7E, 0x00};
  static const uint8_t O[8] = {0x3C, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3C, 0x00};
  static const uint8_t F[8] = {0x7E, 0x40, 0x40, 0x7C, 0x40, 0x40, 0x40, 0x00};
  static const uint8_t R[8] = {0x7C, 0x42, 0x42, 0x7C, 0x48, 0x44, 0x42, 0x00};
  static const uint8_t M[8] = {0x42, 0x66, 0x5A, 0x42, 0x42, 0x42, 0x42, 0x00};
  static const uint8_t K[8] = {0x42, 0x44, 0x48, 0x70, 0x48, 0x44, 0x42, 0x00};
  static const uint8_t N[8] = {0x42, 0x62, 0x52, 0x4A, 0x46, 0x42, 0x42, 0x00};
  static const uint8_t EX[8] = {0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x18, 0x00};
  static const uint8_t SPACE[8] = {0x00, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00};

  switch (c) {
    case 'H':
      return H;
    case 'E':
      return E;
    case 'L':
      return L;
    case 'O':
      return O;
    case 'F':
      return F;
    case 'R':
      return R;
    case 'M':
      return M;
    case 'K':
      return K;
    case 'N':
      return N;
    case '!':
      return EX;
    case ' ':
    default:
      return SPACE;
  }
}

static void draw_char(const struct kuro_framebuffer* fb, uint32_t x, uint32_t y,
                      char c, uint8_t r, uint8_t g, uint8_t b) {
  const uint8_t* glyph = glyph_for(c);
  for (uint32_t row = 0; row < 8; ++row) {
    for (uint32_t col = 0; col < 8; ++col) {
      if (glyph[row] & (uint8_t)(1U << (7U - col))) {
        put_pixel(fb, x + col, y + row, r, g, b);
      }
    }
  }
}

static void draw_text(const struct kuro_framebuffer* fb, uint32_t x, uint32_t y,
                      const char* text, uint8_t r, uint8_t g, uint8_t b) {
  uint32_t pen_x = x;
  for (size_t i = 0; text[i] != '\0'; ++i) {
    draw_char(fb, pen_x, y, text[i], r, g, b);
    pen_x += 8;
  }
}

int _start(const struct kuro_boot_info* boot_info) {
  const struct kuro_framebuffer* fb = &boot_info->framebuffer;

  if (fb->base != 0 && fb->width != 0 && fb->height != 0) {
    // fill_rect(fb, 0, 0, fb->width, fb->height, 0, 0, 0);
    draw_text(fb, 32, 32, "HELLO FROM KERNEL!", 255, 255, 255);

    fill_rect(fb, 100, 100, 100, 100, 255, 0, 0);
  }

  while (1) {
    __asm__("hlt");
  }

  return 42;
}