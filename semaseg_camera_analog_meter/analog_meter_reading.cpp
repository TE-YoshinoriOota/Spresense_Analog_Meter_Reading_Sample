#include "analog_meter_reading.h"

xy_float_point init_p[4];
xy_int_point center;
float magnification_rate = 0.0;


void init_area(int width, int height, int offset, int center_x, int center_y) {
  float sx = (float)(offset) / (offset + width);
  float sy = (height * 0.5) / (offset + width);
  float ey = -sy;
  float ex = 1.0;
  magnification_rate = offset + width;

  init_p[0].x =  sx; init_p[0].y = ey;
  init_p[1].x =  ex; init_p[1].y = ey;
  init_p[2].x =  ex; init_p[2].y = sy;
  init_p[3].x =  sx; init_p[3].y = sy;
  center.x = center_x;  
  center.y = center_y;
}

void convert_points(xy_float_point *fpoints, xy_int_point *points, int array_size) {
  for (int n = 0; n < array_size; ++n) {
    points[n].x = (int16_t)(fpoints[n].x*magnification_rate) + center.x;
    points[n].y = (int16_t)(fpoints[n].y*magnification_rate) + center.y;
  }
}

xy_float_point rotate_point(float degree, xy_float_point point) {
  float rad = degree*PI/180.0;
  xy_float_point ret;
  ret.x = cos(rad)*point.x - sin(rad)*point.y;
  ret.y = sin(rad)*point.x + cos(rad)*point.y;
  return ret;
}


xy_float_point fp[4];
xy_int_point p[4];

xy_float_point* get_xy_float_point() { return fp; }
xy_int_point* get_xy_int_point() { return p; }

rgb_value get_area_value(uint8_t *gfx_buf, float degree) {
  rgb_value rgb;
  rgb.r = 0;  rgb.g = 0;  rgb.b = 0; rgb.sum = 0;

  uint16_t *gfx_buf_16 = (uint16_t*)gfx_buf;

  /* rotate float points by degree */
  for (int n = 0; n < 4; ++n) {
    fp[n] = rotate_point((degree), init_p[n]);
  }

  /* convert float points to int points on the lcd coordinate */
  convert_points(fp, p, 4);

  /************************************************************/
  if (degree >= 0 && degree < 90) {

    /* in case of sy == ey */
    if (p[0].y == p[1].y) {
      for (int y = p[0].y; y < p[3].y; ++y) {
        for (int x = p[0].x; x < p[1].x; ++x) {
          uint16_t value = gfx_buf_16[x+y*WIDTH];
          rgb.r += ((value & 0xF800) >> 11);
          rgb.g += ((value & 0x07E0) >>  5);
          rgb.b +=  (value & 0x001F);
          rgb.sum += 255;
        }
      }
    } else {
      int y = p[0].y;
      float s_delta, e_delta;
      int s_x, e_x;

      s_delta = float(p[3].x - p[0].x) / float(p[3].y - p[0].y);
      e_delta = float(p[1].x - p[0].x) / float(p[1].y - p[0].y);
      for (; y < p[3].y; ++y) {
        s_x = (int)(s_delta*(y - p[0].y)) + p[0].x;
        e_x = (int)(e_delta*(y - p[0].y)) + p[0].x;
        if (s_x < p[3].x)  s_x = p[3].x;
        if (e_x > p[1].x)  e_x = p[1].x;
        for (int x = s_x; x < e_x; ++x) {
          uint16_t value = gfx_buf_16[x+y*WIDTH];
          rgb.r += ((value & 0xF800) >> 11);
          rgb.g += ((value & 0x07E0) >>  5);
          rgb.b +=  (value & 0x001F);
          rgb.sum += 255;          
        }
      } 

      s_delta = float(p[2].x - p[3].x) / float(p[2].y - p[3].y);
      for (; y < p[1].y; ++y) {
        s_x = (int)(s_delta*(y - p[3].y)) + p[3].x;
        e_x = (int)(e_delta*(y - p[0].y)) + p[0].x;
        if (s_x < p[3].x)  s_x = p[3].x;
        if (e_x > p[1].x)  e_x = p[1].x;        
        for (int x = s_x; x < e_x; ++x) {
          uint16_t value = gfx_buf_16[x+y*WIDTH];
          rgb.r += ((value & 0xF800) >> 11);
          rgb.g += ((value & 0x07E0) >>  5);
          rgb.b +=  (value & 0x001F);
          rgb.sum += 255;          
        }
      } 

      e_delta = float(p[2].x - p[1].x) / float(p[2].y - p[1].y);
      for (; y < p[2].y; ++y) {
        s_x = (int)(s_delta*(y - p[3].y)) + p[3].x;
        e_x = (int)(e_delta*(y - p[1].y)) + p[1].x;
        if (s_x < p[3].x)  s_x = p[3].x;
        if (e_x > p[1].x)  e_x = p[1].x;
        for (int x = s_x; x < e_x; ++x) {
          uint16_t value = gfx_buf_16[x+y*WIDTH];
          rgb.r += ((value & 0xF800) >> 11);
          rgb.g += ((value & 0x07E0) >>  5);
          rgb.b +=  (value & 0x001F);
          rgb.sum += 255;          
        }
      } 
    }

  /************************************************************/
  } else if (degree >=  90 && degree < 180) {

    /* in case of sy == ey */
    if (p[2].y == p[3].y) {
      for (int y = p[2].y; y < p[1].y; ++y) {
        for (int x = p[2].x; x < p[3].x; ++x) {
          uint16_t value = gfx_buf_16[x+y*WIDTH];
          rgb.r += ((value & 0xF800) >> 11);
          rgb.g += ((value & 0x07E0) >>  5);
          rgb.b +=  (value & 0x001F);
          rgb.sum += 255;          
        }
      }
    } else {
      int y = p[3].y;
      float s_delta, e_delta;
      int s_x, e_x;

      s_delta = float(p[2].x - p[3].x) / float(p[2].y - p[3].y);
      e_delta = float(p[0].x - p[3].x) / float(p[0].y - p[3].y);
      for (; y < p[0].y; ++y) {
        s_x = (int)(s_delta*(y - p[3].y)) + p[3].x;
        e_x = (int)(e_delta*(y - p[3].y)) + p[3].x;
        if (s_x < p[2].x)  s_x = p[2].x;
        if (e_x > p[0].x)  e_x = p[0].x;
        for (int x = s_x; x < e_x; ++x) {
          uint16_t value = gfx_buf_16[x+y*WIDTH];
          rgb.r += ((value & 0xF800) >> 11);
          rgb.g += ((value & 0x07E0) >>  5);
          rgb.b +=  (value & 0x001F);
          rgb.sum += 255;          
        }
      } 

      e_delta = float(p[1].x - p[0].x) / float(p[1].y - p[0].y);
      for (; y < p[2].y; ++y) {
        s_x = (int)(s_delta*(y - p[3].y)) + p[3].x;
        e_x = (int)(e_delta*(y - p[0].y)) + p[0].x;
        if (s_x < p[2].x)  s_x = p[2].x;
        if (e_x > p[0].x)  e_x = p[0].x;        
        for (int x = s_x; x < e_x; ++x) {
          uint16_t value = gfx_buf_16[x+y*WIDTH];
          rgb.r += ((value & 0xF800) >> 11);
          rgb.g += ((value & 0x07E0) >>  5);
          rgb.b +=  (value & 0x001F);
          rgb.sum += 255;          
        }
      } 

      s_delta = float(p[1].x - p[2].x) / float(p[1].y - p[2].y);
      for (; y < p[1].y; ++y) {
        s_x = (int)(s_delta*(y - p[2].y)) + p[2].x;
        e_x = (int)(e_delta*(y - p[0].y)) + p[0].x;
        if (s_x < p[2].x)  s_x = p[2].x;
        if (e_x > p[0].x)  e_x = p[0].x;
        for (int x = s_x; x < e_x; ++x) {
          uint16_t value = gfx_buf_16[x+y*WIDTH];
          rgb.r += ((value & 0xF800) >> 11);
          rgb.g += ((value & 0x07E0) >>  5);
          rgb.b +=  (value & 0x001F);
          rgb.sum += 255;          
        }
      } 
    }

  /************************************************************/
  } else if (degree >= 180 && degree < 270) {

    if (p[2].y == p[3].y) { 
      for (int y = p[2].y; y < p[1].y; ++y) {
        for (int x = p[2].x; x < p[3].x; ++x) {
          uint16_t value = gfx_buf_16[x+y*WIDTH];
          rgb.r += ((value & 0xF800) >> 11);
          rgb.g += ((value & 0x07E0) >>  5);
          rgb.b +=  (value & 0x001F);
          rgb.sum += 255;          
        }
      }
    } else {
     
      int y = p[2].y;
      float s_delta, e_delta;
      int s_x, e_x;
      s_delta = float(p[1].x - p[2].x) / float(p[1].y - p[2].y);
      e_delta = float(p[3].x - p[2].x) / float(p[3].y - p[2].y);
      for (; y < p[1].y; ++y) {
        s_x = (int)(s_delta*(y - p[2].y)) + p[2].x;
        e_x = (int)(e_delta*(y - p[2].y)) + p[2].x;
        if (s_x < p[1].x)  s_x = p[1].x;
        if (e_x > p[3].x)  e_x = p[3].x;        
        for (int x = s_x; x < e_x; ++x) {
          uint16_t value = gfx_buf_16[x+y*WIDTH];
          rgb.r += ((value & 0xF800) >> 11);
          rgb.g += ((value & 0x07E0) >>  5);
          rgb.b +=  (value & 0x001F);
          rgb.sum += 255;          
        }
      } 

      s_delta = float(p[0].x - p[1].x) / float(p[0].y - p[1].y);
      for (; y < p[3].y; ++y) {
        s_x = (int)(s_delta*(y - p[1].y)) + p[1].x;
        e_x = (int)(e_delta*(y - p[2].y)) + p[2].x;
        if (s_x < p[1].x)  s_x = p[1].x;
        if (e_x > p[3].x)  e_x = p[3].x;        
        for (int x = s_x; x < e_x; ++x) {
          uint16_t value = gfx_buf_16[x+y*WIDTH];
          rgb.r += ((value & 0xF800) >> 11);
          rgb.g += ((value & 0x07E0) >>  5);
          rgb.b +=  (value & 0x001F);
          rgb.sum += 255;          
        }
      } 

      e_delta = float(p[0].x - p[3].x) / float(p[0].y - p[3].y);
      for (; y < p[0].y; ++y) {
        s_x = (int)(s_delta*(y - p[1].y)) + p[1].x;
        e_x = (int)(e_delta*(y - p[3].y)) + p[3].x;
        if (s_x < p[1].x)  s_x = p[1].x;
        if (e_x > p[3].x)  e_x = p[3].x;        
        for (int x = s_x; x < e_x; ++x) {
          uint16_t value = gfx_buf_16[x+y*WIDTH];
          rgb.r += ((value & 0xF800) >> 11);
          rgb.g += ((value & 0x07E0) >>  5);
          rgb.b +=  (value & 0x001F);
          rgb.sum += 255;          
        }
      } 
    }
  
  /************************************************************/
  } else if (degree >= 270 && degree < 360) {   

    if (p[0].y == p[1].y) {
      for (int y = p[0].y; y < p[3].y; ++y) {
        for (int x = p[0].x; x < p[1].x; ++x) {
          uint16_t value = gfx_buf_16[x+y*WIDTH];
          rgb.r += ((value & 0xF800) >> 11);
          rgb.g += ((value & 0x07E0) >>  5);
          rgb.b +=  (value & 0x001F);
          rgb.sum += 255;          
        }
      }
    } else {
     
      int y = p[1].y;
      float s_delta, e_delta;
      int s_x, e_x;
      s_delta = float(p[0].x - p[1].x) / float(p[0].y - p[1].y);
      e_delta = float(p[2].x - p[1].x) / float(p[2].y - p[1].y);
      for (; y < p[2].y; ++y) {
        s_x = (int)(s_delta*(y - p[1].y)) + p[1].x;
        e_x = (int)(e_delta*(y - p[1].y)) + p[1].x;
        if (s_x < p[0].x)  s_x = p[0].x;        
        if (e_x > p[2].x)  e_x = p[2].x;        
        for (int x = s_x; x < e_x; ++x) {
          uint16_t value = gfx_buf_16[x+y*WIDTH];
          rgb.r += ((value & 0xF800) >> 11);
          rgb.g += ((value & 0x07E0) >>  5);
          rgb.b +=  (value & 0x001F);
          rgb.sum += 255;          
        }
      } 

      e_delta = float(p[3].x - p[2].x) / float(p[3].y - p[2].y);
      for (; y < p[0].y; ++y) {
        s_x = (int)(s_delta*(y - p[1].y)) + p[1].x;
        e_x = (int)(e_delta*(y - p[2].y)) + p[2].x;
        if (s_x < p[0].x)  s_x = p[0].x;
        if (e_x > p[2].x)  e_x = p[2].x;
        for (int x = s_x; x < e_x; ++x) {
          uint16_t value = gfx_buf_16[x+y*WIDTH];
          rgb.r += ((value & 0xF800) >> 11);
          rgb.g += ((value & 0x07E0) >>  5);
          rgb.b +=  (value & 0x001F);
          rgb.sum += 255;          
        }
      } 

      s_delta = float(p[3].x - p[0].x) / float(p[3].y - p[0].y);
      for (; y < p[3].y; ++y) {
        s_x = (int)(s_delta*(y - p[0].y)) + p[0].x;
        e_x = (int)(e_delta*(y - p[2].y)) + p[2].x;
        if (s_x < p[0].x)  s_x = p[0].x;        
        if (e_x > p[2].x)  e_x = p[2].x;        
        for (int x = s_x; x < e_x; ++x) {
          uint16_t value = gfx_buf_16[x+y*WIDTH];
          rgb.r += ((value & 0xF800) >> 11);
          rgb.g += ((value & 0x07E0) >>  5);
          rgb.b +=  (value & 0x001F);
          rgb.sum += 255;          
        }
      } 
    }
  }

  return rgb;
}