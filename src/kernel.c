#include "serial.h"
#include "fb.h"
#include "qemu_dma.h"
#include "malloc.h"
#include "trap.h"

void thread_a(void) {
    int e = 0;
    while(1) { kprint("A ");
       ksleep(100); 
       e++;
      if (e == 5) {
        kexit();
      }
      }
}
void thread_b(void) {
    while(1) { kprint("B ");
       ksleep(500); }
}

void kernel_main(void) {
  if (check_fw_cfg_dma()) {
    kprint("guest fw_cfg dma-interface enabled \n");
  } else {
    kprint("guest fw_cfg dma-interface NOT enabled - abort \n");
    return;
  }

  heap_init(HEAP_SIZE);
  kprint("setup heap successfull\n");

  if (ramfb_setup(FB_WIDTH, FB_HEIGHT) != 0) {
    kprint("error setting up ramfb \n");
  }
  kprint("setup ramfb successfull\n");

  init_traps();
  kprint("setup traps & threads successfull\n");

    create_thread(thread_a, 4096);
  create_thread(thread_b, 4096);

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
  void *one = kmalloc(1024);
  void *two = kmalloc(64 * 1024);
  void *three = kmalloc(1024 * 1024);
  
  for (int y = 0; y < FB_HEIGHT; y++) {
    for (int x = 0; x < FB_WIDTH; x++) {
        put_pixel(x, y, 0xFFFFFFFF);
    }
  }
  flush();

  kprint_ui((uint64_t)get_busy_mem_size()); kprint(" now\n");

  kfree(one);
  kfree(two);
  kfree(three);

  kprint_ui((uint64_t)get_busy_mem_size()); kprint(" after big clean\n");
  // display_qoi_image();

  kprint("Starting Scheduler and Traps...!\n");

  while (1) {
      asm volatile("wfi");
  }
}