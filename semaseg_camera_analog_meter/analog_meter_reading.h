#include <math.h>

#define WIDTH  (320)
#define HEIGHT (240)
#define PI (3.14159265)

typedef struct xy_float_point {
  float x;
  float y;
} xy_float_point;

typedef struct xy_int_point {
  int16_t x;
  int16_t y;
} xy_int_point;

typedef struct rgb_value {
  uint32_t r;
  uint32_t g;
  uint32_t b;
  uint32_t sum;
} rgb_value;


void init_area(int width, int height, int offset, int center_x, int center_y);
xy_int_point* get_xy_int_point();
rgb_value get_area_value(uint8_t *gfx_buf, float degree);
