#include "fb.h"
#include "qemu_dma.h"
#include "serial.h"
#include "malloc.h"

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

void memcpy_(void *dest, void *src, uint64_t n) {
   char *csrc = (char *)src;
   char *cdest = (char *)dest;
  
   for (int i=0; i<n; i++) cdest[i] = csrc[i];
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
    memcpy_((void*)fb.fb_addr, (void*)back_buffer, fb.fb_size);
    asm volatile("fence w, o" ::: "memory");
}

void write_xrgb256_pixel(uint16_t x, uint16_t y, uint8_t pixel[4]){ // deprecated
    memcpy_((void*)fb.fb_addr + ((y * fb.fb_stride) + (x * fb.fb_bpp)), pixel, 4);
}


void write_rgb256_pixel(uint16_t x, uint16_t y, uint8_t pixel[3]) { // deprecated
    // offset one byte (xrgb)
    memcpy_((void*)fb.fb_addr + ((y * fb.fb_stride) + (x * fb.fb_bpp)), pixel, 4);
}

void draw_rgb256_map(uint32_t x_res, uint32_t y_res, uint8_t *rgb_map) {

    uint8_t map_bpp = 4;
    uint64_t map_stride = x_res * map_bpp;
    uint64_t map_size = map_stride*y_res;

    uint64_t i = 0;
    for (int map_i = 0; map_i < map_size; map_i += 4) {
        if (map_i%map_stride == 0 && map_i != 0) {
            i += fb.fb_stride-map_stride;
        }
        // 1 compensates for alignement (xRGB)
        memcpy_((void*)fb.fb_addr + i, &rgb_map[map_i], 4);

        i += 4;
    }
} // deprecated
