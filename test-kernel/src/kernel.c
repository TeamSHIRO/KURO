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
#include <stdio.h>

#include "boot_info.h"
#include "font.h"

uint64_t cursor_y = 0;

enum {
  KURO_PIXEL_RGBR_8 = 0,
  KURO_PIXEL_BGRR_8 = 1,
  KURO_PIXEL_BITMASK = 2,
  KURO_MEMORY_PAGE_SIZE = 4096,
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

enum {
  FONT_GLYPH_COLS = 8,
  FONT_GLYPH_ROWS = 13,
  FONT_ADVANCE_X = 10,
  FONT_FIRST_CHAR = 32,
};

static const uint8_t* glyph_for(char c) {
  const size_t glyph_count = sizeof(font) / sizeof(font[0]);
  const unsigned char uc = (unsigned char)c;

  if (uc >= FONT_FIRST_CHAR && (size_t)(uc - FONT_FIRST_CHAR) < glyph_count) {
    return font[uc - FONT_FIRST_CHAR];
  }

  return font[0];
}

static void draw_char(const struct kuro_framebuffer* fb, uint32_t x, uint32_t y,
                      char c, uint8_t r, uint8_t g, uint8_t b) {
  const uint8_t* glyph = glyph_for(c);
  for (uint32_t row = 0; row < FONT_GLYPH_ROWS; ++row) {
    for (uint32_t col = 0; col < FONT_GLYPH_COLS; ++col) {
      if (glyph[row] & (uint8_t)(1U << (7U - col))) {
        put_pixel(fb, x + col, y + (FONT_GLYPH_ROWS - 1U - row), r, g, b);
      }
    }
  }
}

static void draw_text(const struct kuro_framebuffer* fb, uint32_t x, uint32_t y,
                      const char* text, uint8_t r, uint8_t g, uint8_t b) {
  uint32_t pen_x = x;
  for (size_t i = 0; text[i] != '\0'; ++i) {
    draw_char(fb, pen_x, y, text[i], r, g, b);
    pen_x += FONT_ADVANCE_X;
  }
}

static void printLn(const struct kuro_framebuffer* fb, const char* text,
                    uint8_t r, uint8_t g, uint8_t b) {
  draw_text(fb, 0, cursor_y, text, r, g, b);
  cursor_y += FONT_GLYPH_ROWS;
}

static size_t str_len(const char* text) {
  size_t len = 0;

  while (text[len] != '\0') {
    ++len;
  }

  return len;
}

static void reverse_range(char* text, size_t len) {
  for (size_t i = 0; i < len / 2; ++i) {
    char tmp = text[i];
    text[i] = text[len - 1 - i];
    text[len - 1 - i] = tmp;
  }
}

static size_t u64_to_dec(char* buffer, size_t capacity, uint64_t value) {
  size_t len = 0;

  if (capacity == 0) return 0;

  if (value == 0) {
    if (capacity < 2) {
      buffer[0] = '\0';
      return 0;
    }

    buffer[0] = '0';
    buffer[1] = '\0';
    return 1;
  }

  while (value != 0 && len + 1 < capacity) {
    buffer[len++] = (char)('0' + (value % 10));
    value /= 10;
  }

  buffer[len] = '\0';
  reverse_range(buffer, len);
  return len;
}

static void print_label_u64(const struct kuro_framebuffer* fb,
                            const char* label, uint64_t value, uint8_t r,
                            uint8_t g, uint8_t b) {
  char digits[21];
  uint32_t label_x;

  u64_to_dec(digits, sizeof(digits), value);
  draw_text(fb, 0, cursor_y, label, r, g, b);

  label_x = (uint32_t)(str_len(label) * FONT_ADVANCE_X);
  draw_text(fb, label_x, cursor_y, digits, r, g, b);
  cursor_y += FONT_GLYPH_ROWS;
}

static const struct kuro_memory_descriptor* memory_descriptor_at(
    const struct kuro_memory_map* memory_map, uint32_t index) {
  const uint8_t* descriptors =
      (const uint8_t*)(uintptr_t)memory_map->descriptors;

  return (const struct kuro_memory_descriptor*)(descriptors +
                                                ((size_t)index *
                                                 memory_map->descriptor_size));
}

int _start(const struct kuro_boot_info* boot_info) {
  const struct kuro_framebuffer* fb = &boot_info->framebuffer;
  const struct kuro_memory_map* memory_map = &boot_info->memory_map;
  uint64_t total_usable_memory = 0;
  uint64_t first_usable_base = 0;
  uint64_t first_usable_size = 0;

  if (fb->base != 0 && fb->width != 0 && fb->height != 0) {
    // fill_rect(fb, 0, 0, fb->width, fb->height, 0, 0, 0);
    printLn(fb, "HELLO FROM KERNEL! KURO IS WORKING SUCCESSFULLY!", 255, 255,
            0);
    printLn(fb, "Framebuffer Information:", 255, 255, 255);
    print_label_u64(fb, "  Framebuffer base: ", fb->base, 200, 200, 200);
    print_label_u64(fb, "  Framebuffer width: ", fb->width, 200, 200, 200);
    print_label_u64(fb, "  Framebuffer height: ", fb->height, 200, 200, 200);
    printLn(fb, "Memory Information:", 255, 255, 255);

    if (boot_info->magic == KURO_BOOT_INFO_MAGIC &&
        boot_info->version == KURO_BOOT_INFO_VERSION &&
        memory_map->descriptors != 0 && memory_map->descriptor_size != 0) {
      for (uint32_t i = 0; i < memory_map->descriptor_count; ++i) {
        const struct kuro_memory_descriptor* descriptor =
            memory_descriptor_at(memory_map, i);
        const uint64_t region_size =
            descriptor->number_of_pages * KURO_MEMORY_PAGE_SIZE;

        if (descriptor->type != KURO_MEMORY_CONVENTIONAL) {
          continue;
        }

        total_usable_memory += region_size;
        if (first_usable_size == 0) {
          first_usable_base = descriptor->physical_start;
          first_usable_size = region_size;
        }
      }

      print_label_u64(fb, "  Map entries: ", memory_map->descriptor_count, 200,
                      200, 200);
      print_label_u64(fb, "  First usable base: ", first_usable_base, 200, 200,
                      200);
      print_label_u64(fb,
                      "  First usable size (MB): ", first_usable_size / 1000000,
                      200, 200, 200);
      print_label_u64(fb, "  Total usable memory (MB): ",
                      total_usable_memory / 1000000, 200, 200, 200);

      for (uint64_t i = 0; i < sizeof(font) / sizeof(font[0]); ++i) {
        draw_char(fb, i * FONT_ADVANCE_X, cursor_y + 10,
                  (char)(i + FONT_FIRST_CHAR), 255, 255, 255);
      }

      fill_rect(fb, fb->width - 100, fb->height - 100, 100, 100, 255, 255, 255);
    } else {
      printLn(fb, "No valid memory map from bootloader.", 255, 120, 120);
    }
  }

  while (1) {
    __asm__("hlt");
  }

  return 42;
}