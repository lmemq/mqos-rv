#include "fb.h"
#include "qemu_dma.h"
#include "serial.h"
#include "malloc.h"
// #include "wallpaper_data.h"

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

// void display_qoi_image(void) {
//     qoi_desc desc;
//     uint8_t *pixel_data = qoi_decode(imagetest_qoi, imagetest_qoi_len, &desc, 4);
    
//     if (!pixel_data) {
//         return;
//     }

//     uint32_t width = desc.width;
//     uint32_t height = desc.height;

//     uint32_t *screen = (uint32_t *)back_buffer;
//     uint32_t *pixels = (uint32_t *)pixel_data;
    
//     uint32_t total_pixels = width * height;
    
//     uint32_t screen_width = FB_WIDTH;

//     uint32_t draw_width = (desc.width < screen_width) ? desc.width : screen_width;
//     uint32_t draw_height = desc.height; 

//     for (uint32_t y = 0; y < draw_height; y++) {
//         for (uint32_t x = 0; x < draw_width; x++) {
//             uint32_t img_idx = y * desc.width + x;
//             uint32_t scr_idx = y * screen_width + x;

//             uint32_t p = pixels[img_idx];
            
//             uint8_t r = (p >> 0)  & 0xFF;
//             uint8_t g = (p >> 8)  & 0xFF;
//             uint8_t b = (p >> 16) & 0xFF;
//             uint8_t a = (p >> 24) & 0xFF;

//             screen[scr_idx] = (a << 24) | (r << 16) | (g << 8) | b;
//         }
//     }
//     flush();
//     kfree(pixel_data);
// }

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
    kprint("Backbuffer is in heap at ");
    kprint_ui((uint64_t)ptr);
    kprint(" \n");

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
