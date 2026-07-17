#include "fb.h"
#include "qemu_dma.h"
#include "serial.h"
#include "malloc.h"
#include "wallpaper.h"
#include <stdarg.h>

#define ASCII_FONT_IMPLEMENTATION
#include "ascii_font.h" 

#define QOI_NO_STDIO
#define QOI_MALLOC(sz) kmalloc(sz)
#define QOI_FREE(p)    kfree(p)
#define QOI_MEMSET(dest, value, size) kmemset(dest, value, size)
#define QOI_MEMCPY(dest, src, size)   kmemcpy(dest, src, size)
#define QOI_IMPLEMENTATION
#include "qoi.h"

typedef struct {
    uint64_t fb_addr;
    uint32_t fb_width;
    uint32_t fb_height;
    uint32_t fb_bpp;

    uint32_t fb_stride;
    uint32_t fb_size;
} fb_info;

fb_info fb;
uint32_t* back_buffer = 0;

static int term_cx = 10;
static int term_cy = 10;
static int term_start_x = 10;
static int term_scale = 1;

uint64_t disable_interrupts(void) {
    uint64_t mstatus;
    asm volatile("csrrc %0, mstatus, %1" : "=r"(mstatus) : "r"(1ULL << 3));
    return mstatus;
}

void restore_interrupts(uint64_t mstatus) {
    asm volatile("csrw mstatus, %0" :: "r"(mstatus));
}

void fkprintf_set_cursor(int x, int y, int scale) {
    term_cx = x;
    term_cy = y;
    term_start_x = x;
    term_scale = scale;
}

void display_qoi_image(void) {
    qoi_desc desc;
    uint8_t *pixel_data = qoi_decode(wallpaper_qoi, wallpaper_qoi_len, &desc, 4);
    
    if (!pixel_data) {
        return;
    }

    uint32_t width = desc.width;
    uint32_t height = desc.height;

    uint32_t *screen = (uint32_t *)back_buffer;
    uint32_t *pixels = (uint32_t *)pixel_data;
    
    uint32_t total_pixels = width * height;
    
    uint32_t screen_width = FB_WIDTH;

    uint32_t draw_width = (desc.width < screen_width) ? desc.width : screen_width;
    uint32_t draw_height = desc.height; 

    for (uint32_t y = 0; y < draw_height; y++) {
        for (uint32_t x = 0; x < draw_width; x++) {
            uint32_t img_idx = y * desc.width + x;
            uint32_t scr_idx = y * screen_width + x;

            uint32_t p = pixels[img_idx];
            
            uint8_t r = (p >> 0)  & 0xFF;
            uint8_t g = (p >> 8)  & 0xFF;
            uint8_t b = (p >> 16) & 0xFF;
            uint8_t a = (p >> 24) & 0xFF;

            screen[scr_idx] = (a << 24) | (r << 16) | (g << 8) | b;
        }
    }
    kfree(pixel_data);
}

int ramfb_setup(uint32_t width, uint32_t height) {
    extern uint64_t _stack_top;
    uint64_t addr = (uint64_t)&_stack_top + HEAP_SIZE;
    uint32_t bpp = FB_BPP;
    uint32_t stride = bpp * width;
    uint32_t size = stride * height;

    uint32_t select = qemu_cfg_find_file();
    if (select == 0) {
        return 1;
    }

    struct QemuRAMFBCfg cfg = {
        .addr   = __builtin_bswap64(addr),
        .fourcc = __builtin_bswap32(DRM_FORMAT_XRGB8888),
        .flags  = __builtin_bswap32(0),
        .width  = __builtin_bswap32(width),
        .height = __builtin_bswap32(height),
        .stride = __builtin_bswap32(stride),
    };
    qemu_cfg_write_entry(&cfg, select, sizeof(cfg));

    back_buffer = kmalloc(size);
    void* ptr = back_buffer;
    kprintf("Backbuffer is in heap at %p (Size: %u bytes)\n", ptr, FB_WIDTH * FB_HEIGHT * 4);

    fb.fb_addr = addr;
    fb.fb_bpp = bpp;
    fb.fb_height = height;
    fb.fb_size = size; 
    fb.fb_stride = stride;
    fb.fb_width = width;
    return 0;
}

void put_pixel(uint16_t x, uint16_t y, uint32_t color) {
    back_buffer[y * fb.fb_width + x] = color;
}

void flush() {
    kmemcpy((void*)fb.fb_addr, (void*)back_buffer, fb.fb_size);
    asm volatile("fence w, o" ::: "memory");
}

uint64_t get_pixel(int x, int y) {
    if (x < 0 || x >= FB_WIDTH || y < 0 || y >= FB_HEIGHT) {
        return 0;
    }
    return back_buffer[y * FB_WIDTH + x];
}

void fill_rect(int start_x, int start_y, int width, int height, uint32_t color, uint8_t alpha) {
    if (alpha == 0) return;

    uint8_t target_r = (color >> 16) & 0xFF;
    uint8_t target_g = (color >> 8) & 0xFF;
    uint8_t target_b = color & 0xFF;

    int end_x = start_x + width;
    int end_y = start_y + height;

    if (start_x < 0) start_x = 0;
    if (start_y < 0) start_y = 0;
    if (end_x > FB_WIDTH) end_x = FB_WIDTH;
    if (end_y > FB_HEIGHT) end_y = FB_HEIGHT;

    for (int y = start_y; y < end_y; y++) {
        for (int x = start_x; x < end_x; x++) {
            
            if (alpha == 255) {
                put_pixel(x, y, color);
            } else {
                uint32_t bg_color = get_pixel(x, y);

                uint8_t bg_r = (bg_color >> 16) & 0xFF;
                uint8_t bg_g = (bg_color >> 8) & 0xFF;
                uint8_t bg_b = bg_color & 0xFF;

                uint8_t r = (target_r * alpha + bg_r * (255 - alpha)) / 255;
                uint8_t g = (target_g * alpha + bg_g * (255 - alpha)) / 255;
                uint8_t b = (target_b * alpha + bg_b * (255 - alpha)) / 255;

                uint32_t final_color = (0xFF << 24) | (r << 16) | (g << 8) | b;
                put_pixel(x, y, final_color);
            }
        }
    }
}

void draw_font_char(int x, int y, char ch, uint32_t text_color, int scale) {
    if (scale < 1) scale = 1;

    uint8_t target_r = (text_color >> 16) & 0xFF;
    uint8_t target_g = (text_color >> 8) & 0xFF;
    uint8_t target_b = text_color & 0xFF;

    for (int dy = 0; dy < ASCII_FONT_HEIGHT; dy++) {
        for (int dx = 0; dx < ASCII_FONT_WIDTH; dx++) {
            
            uint8_t alpha = ascii_font[(size_t)ch][dy][dx];
            if (alpha == 0) continue;

            for (int sy = 0; sy < scale; sy++) {
                for (int sx = 0; sx < scale; sx++) {
                    int screen_x = x + dx * scale + sx;
                    int screen_y = y + dy * scale + sy;

                    if (alpha == 255) {
                        put_pixel(screen_x, screen_y, text_color);
                    } else {
                        uint32_t bg_color = get_pixel(screen_x, screen_y);
                        
                        uint8_t bg_r = (bg_color >> 16) & 0xFF;
                        uint8_t bg_g = (bg_color >> 8) & 0xFF;
                        uint8_t bg_b = bg_color & 0xFF;

                        uint8_t r = (target_r * alpha + bg_r * (255 - alpha)) / 255;
                        uint8_t g = (target_g * alpha + bg_g * (255 - alpha)) / 255;
                        uint8_t b = (target_b * alpha + bg_b * (255 - alpha)) / 255;

                        uint32_t final_color = (0xFF << 24) | (r << 16) | (g << 8) | b;
                        put_pixel(screen_x, screen_y, final_color);
                    }
                }
            }
        }
    }
}

void draw_font_string(int x, int y, const char* text, uint32_t color, int scale) {
    for (size_t i = 0; text[i] != 0; i++) {
        draw_font_char(x + i * ASCII_FONT_WIDTH * scale, y, text[i], color, scale);
    }
}

void fkputc(char c, uint32_t color) {
    int char_w = ASCII_FONT_WIDTH * term_scale;
    int char_h = ASCII_FONT_HEIGHT * term_scale;

    if (c == '\n') {
        term_cx = term_start_x;
        term_cy += char_h + (4 * term_scale);
        
        if (term_cy + char_h >= FB_HEIGHT) {
            term_cy = term_start_x;
        }
        return;
    }

    if (c == '\r') {
        term_cx = term_start_x;
        return;
    }

    if (term_cx + char_w >= FB_WIDTH) {
        fkputc('\n', color);
    }

    draw_font_char(term_cx, term_cy, c, color, term_scale);
    
    term_cx += char_w;
}

void fkprintf(const char *format, ...) {
    uint64_t status = disable_interrupts();
    va_list args;
    va_start(args, format);

    uint64_t i = 0;
    uint8_t buffer[24]; 
    uint32_t text_color = 0x00FFFFFF;

    while (format[i] != '\0') {
        if (format[i] != '%') {
            fkputc(format[i], text_color);
            i++;
            continue;
        }

        i++; 
        switch (format[i]) {
            case 'c': {
                char c = (char)va_arg(args, int);
                fkputc(c, text_color);
                break;
            }
            case 's': {
                uint8_t *s = va_arg(args, uint8_t*);
                if (s == 0) s = (uint8_t*)"(null)";
                
                for (uint64_t j = 0; s[j] != 0; j++) {
                    fkputc(s[j], text_color);
                }
                break;
            }
            case 'd':
            case 'u': {
                uint64_t num = va_arg(args, uint64_t);
                uitoa(buffer, num, 10); 
                for (uint64_t j = 0; buffer[j] != 0; j++) {
                    fkputc(buffer[j], text_color);
                }
                break;
            }
            case 'p':
            case 'x': {
                uint64_t num = va_arg(args, uint64_t);
                fkputc('0', text_color);
                fkputc('x', text_color);
                uitoa(buffer, num, 16);
                for (uint64_t j = 0; buffer[j] != 0; j++) {
                    fkputc(buffer[j], text_color);
                }
                break;
            }
            case '%': {
                fkputc('%', text_color);
                break;
            }
            default:
                fkputc('%', text_color);
                fkputc(format[i], text_color);
                break;
        }
        i++;
    }

    va_end(args);
    restore_interrupts(status);
}
