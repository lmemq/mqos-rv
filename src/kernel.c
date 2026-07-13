#include "serial.h"
#include "fb.h"
#include "qemu_dma.h"
#include "image_rgb_map.h"
#include "malloc.h"

void kernel_main(void) {
  extern uint64_t _stack_top;
  uint64_t fb_start = (uint64_t)&_stack_top;

  if (check_fw_cfg_dma()) {
    kprint("guest fw_cfg dma-interface enabled \n");
  } else {
    kprint("guest fw_cfg dma-interface NOT enabled - abort \n");
    return;
  }
  
  uint32_t fb_width = 1280;
  uint32_t fb_height = 720;
  uint32_t fb_bpp = 4; // bytes per pixel (not bits)
  uint32_t fb_stride = fb_bpp * fb_width;

  fb_info fb = {
    .fb_addr = fb_start,
    .fb_width = fb_width,
    .fb_height = fb_height,
    .fb_bpp = fb_bpp,

    .fb_stride = fb_stride,
    .fb_size = fb_stride * fb_height,
  };

  uint64_t heap_start = fb_start + fb.fb_size;
  uint64_t heap_size = 1024 * 1024 * 32;
  heap_init(heap_start, heap_size);

  if (ramfb_setup(&fb) != 0) {
    kprint("error setting up ramfb \n");
  }
  kprint("setup ramfb successfull\n");

  kprint("setup heap successfull\n");
  kprint_ui(get_busy_mem_size());
  kprint(" before\n");

  int* arr = (int*)kmalloc(10 * sizeof(int));
  arr[0] = 100;
  kprint_ui(get_busy_mem_size());
  kprint(" then\n");
  kfree(arr);

  kprint_ui(get_busy_mem_size());

  kprint(" after\n");

  void *ptr1 = kmalloc(40);
  kprint_ui((uint64_t)ptr1); 
  kprint(" before\n");
  kfree(ptr1);
    
  void *ptr2 = kmalloc(40);
  kprint_ui((uint64_t)ptr2); 
  kprint(" after\n");
  kfree(ptr2);

  kprint_ui((uint64_t)kmalloc(1024));       kprint(" 1KB\n");
  kprint_ui((uint64_t)kmalloc(64 * 1024));  kprint(" 64KB\n");
  kprint_ui((uint64_t)kmalloc(1024 * 1024)); kprint(" 1MB\n");

  for (int y = 0; y < fb.fb_height; y++) {
    for (int x = 0; x < fb.fb_width; x++) {
        put_pixel(&fb, x, y, 0xFFFFFFFF);
    }
  }

  flush(&fb);

  kprint_ui((uint64_t)get_busy_mem_size()); kprint(" now\n");

  // draw_rgb256_map(&fb, 500, 500, (uint8_t*)&img[0]);

  while (1);
}