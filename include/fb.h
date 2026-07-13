#ifndef FB_H
#define FB_H

#include  <stdint.h>

#define fourcc_code(a, b, c, d) ((uint32_t)(a) | ((uint32_t)(b) << 8) | \
                                 ((uint32_t)(c) << 16) | ((uint32_t)(d) << 24))


#define DRM_FORMAT_RGB565       fourcc_code('R', 'G', '1', '6') /* [15:0] R:G:B 5:6:5 little endian */
#define DRM_FORMAT_RGB888       fourcc_code('R', 'G', '2', '4') /* [23:0] R:G:B little endian */
#define DRM_FORMAT_XRGB8888     fourcc_code('X', 'R', '2', '4') /* [31:0] x:R:G:B 8:8:8:8 little endian */

#define FB_WIDTH 1280
#define FB_HEIGHT 720
#define FB_BPP 4

int ramfb_setup(uint32_t width, uint32_t height);

void put_pixel(uint16_t x, uint16_t y, uint32_t color);
void flush();

#endif