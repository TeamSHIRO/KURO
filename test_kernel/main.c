// #include "main.h"
#include <efi.h>
#include <stddef.h>
#include "niji-pixel-bold-16.h"

typedef struct {
    char k_magic0;
    char k_magic1;
    char k_magic2;
    char k_magic3;
    char k_magic4;
    uint8_t k_version;
    uint16_t k_reserved;
} KuroIdentifier;

typedef struct {
    uint32_t ks_flags;
    char pad[4]; // padding to align the structure to 8 bytes
    uint64_t ks_address;
    uint64_t ks_size;
    uint64_t ks_align;
} KuroSegmentInfo;

typedef struct {
    uint64_t kf_base;
    uint64_t kf_size;
    uint32_t kf_width;
    uint32_t kf_height;
    uint32_t kf_pixels_per_scanline;
    uint32_t kf_pixel_format;
    uint32_t kf_red_mask;
    uint32_t kf_green_mask;
    uint32_t kf_blue_mask;
    uint32_t kf_reserved_mask;
} KuroFramebuffer;

typedef struct {
    KuroIdentifier ke_identifier;
    uint64_t ke_entry_point;
    uint64_t ke_segment_count;
    KuroSegmentInfo *ke_segments;
    uint64_t ke_stack_start;
    uint64_t ke_stack_end;
    uint64_t ke_stack_size;
    KuroFramebuffer ke_framebuffer;
} KuroExecutableInfo;

void to_wchar(const char *src, CHAR16 *dest, size_t max_len) {
    size_t i = 0;
    while (src[i] != '\0' && i < max_len - 1) {
        dest[i] = (CHAR16) src[i];
        i++;
    }
    dest[i] = L'\0';
}

enum {
    KURO_PIXEL_RGBR_8 = 0,
    KURO_PIXEL_BGRR_8 = 1,
    KURO_PIXEL_BITMASK = 2,
    KURO_MEMORY_PAGE_SIZE = 4096,
    KURO_COLOR_MAX = 255,
    KURO_MASK_MAX_BIT = 31,
    KURO_PIXEL_SHIFT_G = 8U,
    KURO_PIXEL_SHIFT_B = 16U,
    KURO_RETURN_MAGIC_NUMBER = 42,
};

static uint32_t lsb_index(uint32_t mask) {
    uint32_t idx = 0;
    if (mask == 0) {
        return 0;
    }
    while (((mask >> idx) & 1U) == 0U && idx < (uint32_t) KURO_MASK_MAX_BIT) {
        idx++;
    }
    return idx;
}

static uint32_t encode_pixel(const KuroFramebuffer *fb, uint8_t r, uint8_t g, uint8_t b) {
    if (fb->kf_pixel_format == KURO_PIXEL_RGBR_8) {
        return ((uint32_t) r) | ((uint32_t) g << KURO_PIXEL_SHIFT_G) | ((uint32_t) b << KURO_PIXEL_SHIFT_B);
    }

    if (fb->kf_pixel_format == KURO_PIXEL_BGRR_8) {
        return ((uint32_t) b) | ((uint32_t) g << KURO_PIXEL_SHIFT_G) | ((uint32_t) r << KURO_PIXEL_SHIFT_B);
    }

    if (fb->kf_pixel_format == KURO_PIXEL_BITMASK) {
        uint32_t out = 0;
        out |= ((uint32_t) r << lsb_index(fb->kf_red_mask)) & fb->kf_red_mask;
        out |= ((uint32_t) g << lsb_index(fb->kf_green_mask)) & fb->kf_green_mask;
        out |= ((uint32_t) b << lsb_index(fb->kf_blue_mask)) & fb->kf_blue_mask;
        return out;
    }

    return ((uint32_t) r) | ((uint32_t) g << KURO_PIXEL_SHIFT_G) | ((uint32_t) b << KURO_PIXEL_SHIFT_B);
}

static void put_pixel(const KuroFramebuffer *fb, uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b) {
    if (x >= fb->kf_width || y >= fb->kf_height) {
        return;
    }
    volatile uint32_t *pixels = (volatile uint32_t *) (uintptr_t) fb->kf_base;
    const uint64_t offset = ((uint64_t) y * (uint64_t) fb->kf_pixels_per_scanline) + (uint64_t) x;
    pixels[offset] = encode_pixel(fb, r, g, b);
}

static void fill_rect(const KuroFramebuffer *fb, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint8_t r, uint8_t g,
                      uint8_t b) {
    for (uint32_t yy = 0; yy < h; ++yy) {
        for (uint32_t xx = 0; xx < w; ++xx) {
            put_pixel(fb, x + xx, y + yy, r, g, b);
        }
    }
}

enum {
    FONT_GLYPH_COLS = 8,
    FONT_GLYPH_ROWS = 16,
    FONT_ADVANCE_X = 10,
    FONT_FIRST_CHAR = 32,
    FONT_GLYPH_MSBIT = 7U,
    DECIMAL_BASE = 10,
    DIGITS_BUFFER_SIZE = 21
};

uint64_t cursor_y = 0;

static const uint8_t *glyph_for(char c) {
    const size_t glyph_count = sizeof(font) / sizeof(font[0]);
    const unsigned char uc = (unsigned char) c;

    if (uc >= FONT_FIRST_CHAR && (size_t) (uc - FONT_FIRST_CHAR) < glyph_count) {
        return font[uc - FONT_FIRST_CHAR];
    }

    return font[0];
}

static void draw_char(const KuroFramebuffer *framebuffer, uint32_t x, uint32_t y, char c, uint8_t r, uint8_t g,
                      uint8_t b) {
    const uint8_t *glyph = glyph_for(c);
    for (uint32_t row = 0; row < FONT_GLYPH_ROWS; ++row) {
        for (uint32_t col = 0; col < FONT_GLYPH_COLS; ++col) {
            if (glyph[row] & (uint8_t) (1U << (FONT_GLYPH_MSBIT - col))) {
                put_pixel(framebuffer, x + col, y + (FONT_GLYPH_ROWS - 1U - row), r, g, b);
            }
        }
    }
}

static void draw_text(const KuroFramebuffer *framebuffer, uint32_t x, uint32_t y, const char *text, uint8_t r,
                      uint8_t g, uint8_t b) {
    uint32_t pen_x = x;
    for (size_t i = 0; text[i] != '\0'; ++i) {
        draw_char(framebuffer, pen_x, y, text[i], r, g, b);
        pen_x += FONT_ADVANCE_X;
    }
}

static void printLn(const KuroFramebuffer *framebuffer, const char *text, uint8_t r, uint8_t g, uint8_t b) {
    draw_text(framebuffer, 0, cursor_y, text, r, g, b);
    cursor_y += FONT_GLYPH_ROWS;
}

static size_t str_len(const char *text) {
    size_t len = 0;

    while (text[len] != '\0') {
        ++len;
    }

    return len;
}

static void reverse_range(char *text, size_t len) {
    for (size_t i = 0; i < len / 2; ++i) {
        char tmp = text[i];
        text[i] = text[len - 1 - i];
        text[len - 1 - i] = tmp;
    }
}

_Noreturn void _start(KuroExecutableInfo *exec_info, void *data, const char *boot_id) {
    const KuroFramebuffer *fb = &exec_info->ke_framebuffer;

    fill_rect(fb, 0, 0, fb->kf_width, fb->kf_height, 0, 0, 0);

    printLn(fb, "Hello, KURO!", 255, 255, 255);

    // THE RED SQUARE

    fill_rect(fb, 100, 100, 100, 100, 255, 0, 0);

    while (1) {
    }
}
