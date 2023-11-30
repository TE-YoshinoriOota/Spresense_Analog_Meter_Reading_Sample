/*
 *  semaseg_camera.ino - Binary Sematic Segmentation sample
 *  Copyright 2022 Sony Semiconductor Solutions Corporation
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <Camera.h>
#include "Adafruit_ILI9341.h"
#include <DNNRT.h>
#include "analog_meter_reading.h"
#include <SDHCI.h>
#include <Flash.h>

/* For Extension Board */
//#define EXTENSION_BOARD_SPI
#ifdef EXTENSION_BOARD_SPI
#define TFT_DC  9
#define TFT_CS  -1
#define TFT_RST 10
#define SPI_CLOCK 40000000
Adafruit_ILI9341 display = Adafruit_ILI9341(&SPI, TFT_DC, TFT_CS, TFT_RST);
#endif

/* For Main Board */
//#define MAIN_BOARD_SPI5
#ifdef  MAIN_BOARD_SPI5
#define TFT_RST 18
#define TFT_DC  25
#define TFT_CS  -1
Adafruit_ILI9341 tft = Adafruit_ILI9341(&SPI5 ,TFT_DC ,TFT_CS ,TFT_RST);
#endif

/* For LTE-M Extension Board */
//#define LTE_BOARD_SPI3
#ifdef LTE_BOARD_SPI3
#define TFT_DC  6
#define TFT_CS  -1
#define TFT_RST 2
#define SPI_CLOCK 6500000
Adafruit_ILI9341 display = Adafruit_ILI9341(&SPI3, TFT_DC, TFT_CS, TFT_RST);
#endif

#define LTE_BOARD_SPI5
#ifdef LTE_BOARD_SPI5
#define TFT_DC  6
#define TFT_CS  -1
#define TFT_RST 2
#define SPI_CLOCK 20000000
Adafruit_ILI9341 display = Adafruit_ILI9341(&SPI5, TFT_DC, TFT_CS, TFT_RST);
#endif

#define OFFSET_X  (32)
#define OFFSET_Y  (24)
#define DNN_WIDTH  (64)
#define DNN_HEIGHT  (48)
#define CLIP_WIDTH (256)
#define CLIP_HEIGHT  (192)

#define DO_CALIBRATION
//#define DISPLAY_SEARCH

//#define USE_SD_CARD
#ifdef USE_SD_CARD
SDClass SD;
#endif

DNNRT dnnrt;
// RGBの画像を入力
DNNVariable input(DNN_WIDTH*DNN_HEIGHT*3);  

// メーター基準点測定開始ボタン
const int measureTempButton = 9;
const int measureHumdButton = 5;
bool measuredBlueDot = false;
bool measuredGreenDot = false;

// キャリブレーションの幅
const int carib = 5;

int sx0, width0, sy0, height0; // reference point for bluedot
int sx1, width1, sy1, height1; // reference point for greendot


bool detect_center_by_nnb(String model_name, int *sx, int *sy, int *width, int *height) {
#ifdef USE_SD_CARD
    File nnbfile = SD.open(model_name);
#else
    File nnbfile = Flash.open(model_name);
#endif
    if (!nnbfile) {
      Serial.println("nnb not found: -> " + model_name);
      return false;
    }

    Serial.println("DNN initialize");
    int ret = dnnrt.begin(nnbfile);
    if (ret < 0) {
      Serial.print("Runtime initialization failure. ");
      Serial.println(ret);
      dnnrt.end();
      return false;
    }
    nnbfile.close();

    // 推論を実行
    dnnrt.inputVariable(input, 0);
    dnnrt.forward();
    DNNVariable output = dnnrt.outputVariable(0); 
        
    // 認識対象の横幅と横方向座標を取得
    bool err;
    int16_t s_sx, s_width;
    err = get_sx_and_width_of_region(output, DNN_WIDTH, DNN_HEIGHT, &s_sx, &s_width);
    
    // 認識対象の縦幅と縦方向座標を取得
    int16_t s_sy, s_height;
    sx0 = width0 = sy0 = height0 = 0;
    err = get_sy_and_height_of_region(output, DNN_WIDTH, DNN_HEIGHT, &s_sy, &s_height);
    if (!err) {
      Serial.println("detection error");
      dnnrt.end();     
      return false;
    }
    
    // 何も検出できなかった
    if (s_width == 0 || s_height == 0) {
      Serial.println("no detection");
      dnnrt.end();        
      return false;
    } else {
      // 認証対象のボックスと座標をカメラ画像にあわせて拡大
      *sx = s_sx * (CLIP_WIDTH/DNN_WIDTH) + OFFSET_X;
      *width = s_width * (CLIP_WIDTH/DNN_WIDTH);
      *sy = s_sy * (CLIP_HEIGHT/DNN_HEIGHT) + OFFSET_Y;
      *height = s_height * (CLIP_HEIGHT/DNN_HEIGHT);
    }

    dnnrt.end();
    return true;
}

void measure_meter_position(uint8_t* buf, int length, int width, int offset, int x, int y, int start_deg, int end_deg, int *result_deg, xy_int_point* result_p) {

  const int carib = 3; // need to change when you need
  int cx = 0; int cy = 0;
  xy_int_point p[4];

#ifdef DO_CALIBRATION
  Serial.println("Caribrating the center");

  float global_value = 1.0;
  for (int j = y-carib; j <= y+carib; ++j) {
    for (int i = x-carib; i <= x+carib; ++i) {

      init_area(length, width, offset, i, j);         //init_area(30, 3, 15, i, j);

      float min_value = 1.0;
      for (int deg = start_deg; deg < end_deg; ++deg) {
        rgb_value rgb = get_area_value(buf, (float)deg);
        float value = float(rgb.r + rgb.b + rgb.b)/(rgb.sum*3.);
        if (value < min_value) min_value = value;
      }

      if (min_value < global_value) {
        global_value = min_value;
        cx = i;
        cy = j;
        Serial.println("cx: " + String(cx) + " cy: " + String(cy));
        Serial.println("value: " + String(global_value));
      }
    }
  }
#endif

  Serial.println("Search analog meter");

  init_area(length, width, offset, cx, cy);         //init_area(30, 3, 15, i, j);

  float min_value = 1.0;
  for (int deg = start_deg; deg < end_deg; ++deg) {

    rgb_value rgb = get_area_value(buf, (float)deg);
    float value = float(rgb.r + rgb.b + rgb.b)/(rgb.sum*3.);
    Serial.println(String(deg) + ", " + String(value));
    xy_int_point *p = get_xy_int_point();

#ifdef DISPLAY_SEARCH
    display.drawRGBBitmap(0, 0, (uint16_t*)buf, 320, 240);
    display.drawLine(p[0].x, p[0].y, p[1].x, p[1].y, ILI9341_RED);
    display.drawLine(p[1].x, p[1].y, p[2].x, p[2].y, ILI9341_RED);
    display.drawLine(p[2].x, p[2].y, p[3].x, p[3].y, ILI9341_RED);
    display.drawLine(p[3].x, p[3].y, p[0].x, p[0].y, ILI9341_RED);
#endif
    if (value < min_value) {
      min_value = value;
      *result_deg = deg;
      memcpy(result_p, p, sizeof(xy_int_point)*4);
    }              
  }  
}


void CamCB(CamImage img) {

  if (!img.isAvailable()) {
    Serial.println("picture error");
    return;
  }

  // カメラ画像の上下左右反転　（カメラが逆さまのため）
  uint16_t *buf = (uint16_t*)img.getImgBuff();
  static uint8_t tmp0[WIDTH*2];
  static uint8_t tmp1[WIDTH*2];
  for (int y = 0; y < HEIGHT/2; ++y) {
    memcpy(&tmp1[0], &buf[y*WIDTH], sizeof(uint16_t)*WIDTH);
    memcpy(&buf[y*WIDTH], &buf[(HEIGHT-y-1)*WIDTH], sizeof(uint16_t)*WIDTH);
    memcpy(&buf[(HEIGHT-y-1)*WIDTH], &tmp1[0], sizeof(uint16_t)*WIDTH);
    memcpy(&tmp0[0], &buf[y*WIDTH], sizeof(uint16_t)*WIDTH);
    uint8_t *buf0 = (uint8_t*)&buf[y*WIDTH];
    uint8_t *buf1 = (uint8_t*)&buf[(HEIGHT-y-1)*WIDTH];
    for (int x = 0; x < WIDTH*2; x += 2) {
      buf0[WIDTH*2-(x-1)-1] = tmp0[x];
      buf0[WIDTH*2-(x-0)-1] = tmp0[x+1];
      buf1[WIDTH*2-(x-1)-1] = tmp1[x];
      buf1[WIDTH*2-(x-0)-1] = tmp1[x+1];
    }
  }

  // 画像の切り出しと縮小
  CamImage small; 
  CamErr camErr = img.clipAndResizeImageByHW(small
            ,OFFSET_X ,OFFSET_Y 
            ,OFFSET_X+CLIP_WIDTH-1 ,OFFSET_Y+CLIP_HEIGHT-1 
            ,DNN_WIDTH ,DNN_HEIGHT);
  if (!small.isAvailable()) {
    Serial.println("Crop and resize error");
    return;
  }

  // 画像をYUVからRGB565に変換
  small.convertPixFormat(CAM_IMAGE_PIX_FMT_RGB565); 
  uint16_t* sbuf = (uint16_t*)small.getImgBuff();

  // RGBのピクセルをフレームに分割
  float* fbuf_r = input.data();
  float* fbuf_g = fbuf_r + DNN_WIDTH*DNN_HEIGHT;
  float* fbuf_b = fbuf_g + DNN_WIDTH*DNN_HEIGHT;
  for (int i = 0; i < DNN_WIDTH*DNN_HEIGHT; ++i) {
    fbuf_r[i] = (float)((sbuf[i] >> 11) & 0x1F)/31.0; // 0x1F = 31
    fbuf_g[i] = (float)((sbuf[i] >>  5) & 0x3F)/63.0; // 0x3F = 64
    fbuf_b[i] = (float)((sbuf[i])       & 0x1F)/31.0; // 0x1F = 31
  }

detection:
  /* メーターのセンターを検出 */
  if (digitalRead(measureTempButton) == LOW) {
    // Blue dot を検出
    bool ret = detect_center_by_nnb("model.nnb", &sx0, &sy0, &width0, &height0);
    if (ret == false) {
      Serial.println("Blue Dot Detection Error");
      return;
    }
    measuredBlueDot = true;
  }

  if (digitalRead(measureHumdButton) == LOW) {
    // Green dot を検出
    bool ret = detect_center_by_nnb("model2.nnb", &sx1, &sy1, &width1, &height1);
    if (ret == false) {
      Serial.println("Green Dot Detection Error");
      return;
    }
    measuredGreenDot = true;
  }
    
measure:
  int temp_deg = 0;
  int humd_deg = 0;
  xy_int_point temp_p[4];
  xy_int_point humd_p[4];

  img.convertPixFormat(CAM_IMAGE_PIX_FMT_RGB565);  

  // 認識対象のボックスをカメラ画像に描画
  if (measuredBlueDot) {

    draw_box((uint16_t*)img.getImgBuff(), sx0, sy0, width0, height0);
    
    int cx = sx0 + width0/2;
    int cy = sy0 + height0/2;
    Serial.println(" x : " + String(cx));
    Serial.println(" y : " + String(cy));

    int length = 30;
    int width  = 3;
    int offset = 15;
    int start_deg = 180;
    int end_deg = 360;

    measure_meter_position(img.getImgBuff(), length, width, offset, cx, cy, start_deg, end_deg, &temp_deg, temp_p);

  }


  // 認識対象のボックスをカメラ画像に描画
  if (measuredGreenDot) {

    draw_box((uint16_t*)img.getImgBuff(), sx1, sy1, width1, height1);
    
    int cx = sx1 + width1/2;
    int cy = sy1 + height1/2;
    Serial.println(" x : " + String(cx));
    Serial.println(" y : " + String(cy));

    int length = 22;
    int width  = 3;
    int offset = 14;
    int start_deg = 180;
    int end_deg = 360;

    measure_meter_position(img.getImgBuff(), length, width, offset, cx, cy, start_deg, end_deg, &humd_deg, humd_p);
  } 

  display.drawRGBBitmap(0, 0, (uint16_t*)img.getImgBuff(), 320, 240);

  if (measuredBlueDot) {
    float temp_val = float(temp_deg - 180.)*(50.+15.)/(360.-180.) - 15.;
    display.drawLine(temp_p[0].x, temp_p[0].y, temp_p[1].x, temp_p[1].y, ILI9341_BLACK);
    display.drawLine(temp_p[1].x, temp_p[1].y, temp_p[2].x, temp_p[2].y, ILI9341_BLACK);
    display.drawLine(temp_p[2].x, temp_p[2].y, temp_p[3].x, temp_p[3].y, ILI9341_BLACK);
    display.drawLine(temp_p[3].x, temp_p[3].y, temp_p[0].x, temp_p[0].y, ILI9341_BLACK);
    display.setCursor(0, 0);    
    display.fillRect(0,0,320,30,ILI9341_BLACK);
    display.setTextColor(ILI9341_YELLOW);  display.setTextSize(2);
    display.println("Temp: " + String(temp_val) + "  deg: " + String(temp_deg));
    Serial.println("Temp: " + String(temp_val) + "  deg: " + String(temp_deg));
    measuredBlueDot = false;
    delay(5000);
  }

  if (measuredGreenDot) {
    float humd_val = float(humd_deg - 180)*100./(360.-180.);    
    display.drawLine(humd_p[0].x, humd_p[0].y, humd_p[1].x, humd_p[1].y, ILI9341_BLACK);
    display.drawLine(humd_p[1].x, humd_p[1].y, humd_p[2].x, humd_p[2].y, ILI9341_BLACK);
    display.drawLine(humd_p[2].x, humd_p[2].y, humd_p[3].x, humd_p[3].y, ILI9341_BLACK);
    display.drawLine(humd_p[3].x, humd_p[3].y, humd_p[0].x, humd_p[0].y, ILI9341_BLACK);
    display.fillRect(0,0,320,30,ILI9341_BLACK);
    display.setCursor(0, 0);   
    display.setTextColor(ILI9341_GREEN);  display.setTextSize(2);
    display.println("Humid: " + String(humd_val) + "  deg: " + String(humd_deg));
    Serial.println("Humid: " + String(humd_val) + "  deg: " + String(humd_deg));    
    measuredGreenDot = false;      
    delay(5000);
  }

}


void setup() {

  Serial.begin(115200);
  pinMode(measureTempButton, INPUT_PULLUP);
  pinMode(measureHumdButton, INPUT_PULLUP);
  display.begin(SPI_CLOCK);
  display.setRotation(3);
  display.fillRect(0, 0, 320, 240, ILI9341_BLUE);

#ifdef USE_SD_CARD
  while(!SD.begin()) {
    Serial.println("Insert SD Card");
    sleep(1);
  }
#endif

  Serial.println("Camera start");
  theCamera.begin();
  Serial.println("Set Auto white balance parameter");
  theCamera.setAutoWhiteBalanceMode(CAM_WHITE_BALANCE_DAYLIGHT);

  theCamera.startStreaming(true, CamCB);
}

void loop() {
  // put your main code here, to run repeatedly:

}
