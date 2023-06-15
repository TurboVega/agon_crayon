// di_video_buffer.cpp - Function definitions for painting video scan lines
//
// A a video buffer is a set of 1-pixel-high video scan lines that are equal
// in length (number of pixels) to the width of the video screen.
//
// Copyright (c) 2023 Curtis Whitley
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 

#include <string.h>
#include "di_video_buffer.h"
#include "di_set_pixel.h"
#include "di_vert_line.h"
#include "di_horiz_line.h"
#include "di_diag_right_line.h"
#include "di_diag_left_line.h"
#include "di_bitmap.h"
#include "di_tile_map.h"
#include "esp_heap_caps.h"

#define _COMPILE_HEX_DATA_
#define __root /**/
#include "samples\\SKY.h"
#include "samples\\CLOUD.h"
#include "samples\\GRASS.h"
#include "samples\\WALL.h"
#include "samples\\TEST_BITMAP.h"

// dummy data for testing only
uint32_t sdx[10] = {150,199,225,287,333,378,425,506,583,601};
uint32_t sdy[10] = {150,166,203,270,315,356,427,251,335,159};
uint8_t sdcolor[10] = {
  SYNCS_OFF | MASK_RGB(3,0,0),
  SYNCS_OFF | MASK_RGB(0,3,0),
  SYNCS_OFF | MASK_RGB(0,2,3),
  SYNCS_OFF | MASK_RGB(1,2,2),
  SYNCS_OFF | MASK_RGB(0,1,0),
  SYNCS_OFF | MASK_RGB(3,0,1),
  SYNCS_OFF | MASK_RGB(2,0,2),
  SYNCS_OFF | MASK_RGB(2,2,0),
  SYNCS_OFF | MASK_RGB(0,2,2),
  SYNCS_OFF | MASK_RGB(1,2,3)
};

/*#define STAR_PADDING 200
#define NUM_STARS (ACT_LINES+STAR_PADDING)
DiSetPixel g_stars[NUM_STARS];

DiHorizontalLine g_vert_center(300, 300, 200, MASK_RGB(3,0,0));
DiVerticalLine g_horiz_center(CENTER_X, CENTER_Y-100, 200, MASK_RGB(0,0,3));
DiDiagonalRightLine g_diamond_ne(CENTER_X, CENTER_Y-HALF_DIAMOND_SIZE, HALF_DIAMOND_SIZE,  MASK_RGB(2,3,1));
DiDiagonalRightLine g_diamond_sw(CENTER_X-HALF_DIAMOND_SIZE, CENTER_Y, HALF_DIAMOND_SIZE,  MASK_RGB(2,3,1));
DiDiagonalLeftLine g_diamond_nw(CENTER_X, CENTER_Y-HALF_DIAMOND_SIZE, HALF_DIAMOND_SIZE,  MASK_RGB(2,3,1));
DiDiagonalLeftLine g_diamond_se(CENTER_X+HALF_DIAMOND_SIZE-1, CENTER_Y, HALF_DIAMOND_SIZE,  MASK_RGB(2,3,1));
*/

#define NR 5
#define NC 1

DiSetPixel g_x_pixel[5];
DiSetPixel g_y_pixel[5];
DiOpaqueBitmap* gp_opaque_bitmap[NR][NC];

/*DiMaskedBitmap* gp_masked_bitmap4;
DiMaskedBitmap* gp_masked_bitmap5;
DiMaskedBitmap* gp_masked_bitmap6;
DiMaskedBitmap* gp_masked_bitmap7;*/

/*DiOpaqueBitmap* gp_digit_bitmap[10];
DiOpaqueBitmap* gp_value_bitmap[4][6];

DiMaskedBitmap* gp_masked_bitmap = new(64,64) DiMaskedBitmap(64,64);

static const char* digit_data =
" *** "
"*   *"
"*  **"
"* * *"
"**  *"
"*   *"
" *** "

"  *  "
" **  "
"  *  "
"  *  "
"  *  "
"  *  "
"*****"

" *** "
"*   *"
"    *"
" *** "
"*    "
"*    "
"*****"

" *** "
"*   *"
"    *"
" *** "
"    *"
"*   *"
" *** "

"   * "
"  ** "
" * * "
"*  * "
"*****"
"   * "
"   * "

"*****"
"*    "
"*    "
"**** "
"    *"
"*   *"
" *** "

" *** "
"*   *"
"*    "
"**** "
"*   *"
"*   *"
" *** "

"*****"
"*   *"
"   * "
"  *  "
"  *  "
"  *  "
"  *  "

" *** "
"*   *"
"*   *"
" *** "
"*   *"
"*   *"
" *** "

" *** "
"*   *"
"*   *"
" ****"
"    *"
"*   *"
" *** ";
*/
/*#define TILES_ACROSS 20
#define TILES_DOWN   20

DiOpaqueBitmap* tile;*/

#define SKY_BITMAP   0
#define CLOUD_BITMAP 1
#define GRASS_BITMAP 2
#define WALL_BITMAP  3

DiTileMap* tile_map;

/*void breakdown_value(uint32_t value, DiOpaqueBitmap** digits) {
  uint32_t d5 = value/100000; value=value%100000;
  uint32_t d4 = value/10000; value=value%10000;
  uint32_t d3 = value/1000; value=value%1000;
  uint32_t d2 = value/100; value=value%100;
  uint32_t d1 = value/10; value=value%10;
  uint32_t d0 = value;

  digits[5] = gp_digit_bitmap[d5];
  digits[4] = gp_digit_bitmap[d4];
  digits[3] = gp_digit_bitmap[d3];
  digits[2] = gp_digit_bitmap[d2];
  digits[1] = gp_digit_bitmap[d1];
  digits[0] = gp_digit_bitmap[d0];
}*/

void init_stars() {
/*  srand(42);
  for (int i = 0; i < NUM_STARS; i++) {
    int32_t x = rand() % (ACT_PIXELS * 3 / 2) - (ACT_PIXELS / 4);
    int8_t c = (int8_t)(rand() % 63 + 1);
    g_stars[i].m_x = x;
    g_stars[i].m_y = i-(STAR_PADDING/2);
    g_stars[i].m_color = c | SYNCS_OFF;
  }*/

  g_x_pixel[0].m_x = 100;
  g_x_pixel[0].m_y = 98;
  g_x_pixel[0].m_color = 0x14;

  g_x_pixel[1].m_x = 200;
  g_x_pixel[1].m_y = 98;
  g_x_pixel[1].m_color = 0x14;

  g_x_pixel[2].m_x = 300;
  g_x_pixel[2].m_y = 98;
  g_x_pixel[2].m_color = 0x14;

  g_x_pixel[3].m_x = 400;
  g_x_pixel[3].m_y = 98;
  g_x_pixel[3].m_color = 0x14;

  g_x_pixel[4].m_x = 500;
  g_x_pixel[4].m_y = 98;
  g_x_pixel[4].m_color = 0x14;

  g_y_pixel[0].m_x = 98;
  g_y_pixel[0].m_y = 100;
  g_y_pixel[0].m_color = 0x14;

  g_y_pixel[1].m_x = 98;
  g_y_pixel[1].m_y = 200;
  g_y_pixel[1].m_color = 0x14;

  g_y_pixel[2].m_x = 98;
  g_y_pixel[2].m_y = 300;
  g_y_pixel[2].m_color = 0x14;

  g_y_pixel[3].m_x = 98;
  g_y_pixel[3].m_y = 400;
  g_y_pixel[3].m_color = 0x14;

  g_y_pixel[4].m_x = 98;
  g_y_pixel[4].m_y = 500;
  g_y_pixel[4].m_color = 0x14;

/*  for (uint32_t d = 0; d < 10; d++) {
    gp_digit_bitmap[d] = new(5,7) DiOpaqueBitmap(5,7);
    for (int32_t y=0;y<7;y++) {
      for (int32_t x=0;x<5;x++) {
        gp_digit_bitmap[d]->set_opaque_pixel(x, y, (uint8_t)(digit_data[d*(5*7) + y*5 + x]==' '?0x3F:0x00));
      }
    }
  }
*/
  for (uint32_t r = 0; r < NR; r++) {
    for (uint32_t c = 0; c < NC; c++) {
      gp_opaque_bitmap[r][c] = new(64,64) DiOpaqueBitmap(64,64);
    }
  }

  /*gp_masked_bitmap4 = new(64,64) DiMaskedBitmap(64,64);
  gp_masked_bitmap5 = new(64,64) DiMaskedBitmap(64,64);
  gp_masked_bitmap6 = new(64,64) DiMaskedBitmap(64,64);
  gp_masked_bitmap7 = new(64,64) DiMaskedBitmap(64,64);*/

  for (uint32_t r = 0; r < NR; r++) {
    for (uint32_t c = 0; c < NC; c++) {
      gp_opaque_bitmap[r][c]->set_position(-r, r*100+100+r);
      //gp_opaque_bitmap[r][c]->set_position(c*100+100+c, r*100+100+r);
      //gp_opaque_bitmap[r][c]->set_position(r-32, r*100+100+r);
    }
  }

  /*gp_masked_bitmap4->set_position(400,100);
  gp_masked_bitmap5->set_position(501,201);
  gp_masked_bitmap6->set_position(603,303);
  gp_masked_bitmap7->set_position(703,403);*/

  //gp_masked_bitmap->set_position(500,200);

  for (int32_t y=0;y<64;y++) {
    for (int32_t x=0;x<64;x++) {

      for (uint32_t r = 0; r < NR; r++) {
        for (uint32_t c = 0; c < NC; c++) {
          gp_opaque_bitmap[r][c]->set_opaque_pixel(x, y, gtest_bitmapData[y*64+x]);
        }
      }

      /*gp_masked_bitmap4->set_masked_pixel(x, y, gtest_bitmapData[y*64+x]);
      gp_masked_bitmap5->set_masked_pixel(x, y, gtest_bitmapData[y*64+x]);
      gp_masked_bitmap6->set_masked_pixel(x, y, gtest_bitmapData[y*64+x]);
      gp_masked_bitmap7->set_masked_pixel(x, y, gtest_bitmapData[y*64+x]);*/

      //gp_masked_bitmap->set__masked_pixel(x, y, gtest_bitmapData[y*64+x]);
    }
  }

/*
  breakdown_value(heap_caps_get_free_size(MALLOC_CAP_32BIT|MALLOC_CAP_8BIT|MALLOC_CAP_INTERNAL), gp_value_bitmap[0]);
  breakdown_value(heap_caps_get_largest_free_block(MALLOC_CAP_32BIT|MALLOC_CAP_8BIT|MALLOC_CAP_INTERNAL), gp_value_bitmap[1]);
  breakdown_value(heap_caps_get_free_size(MALLOC_CAP_INTERNAL), gp_value_bitmap[2]);
  breakdown_value(heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL), gp_value_bitmap[3]);
*/
  /*tile = new(40,30) DiOpaqueBitmap(40,30);
  for (int32_t y=0;y<30;y++) {
    for (int32_t x=0;x<40;x++) {
      tile->set_pixel(x, y, gtest_bitmapData[y*64+x]);
    }
  }*/
/*
  tile_map = new DiTileMap(4, 20, 20, 40, 30);
  tile_map->clear();
  for (int32_t y=0;y<30;y++) {
    for (int32_t x=0;x<40;x++) {
      tile_map->set_pixel(SKY_BITMAP, x, y, gSKYData[y*40+x]);
      tile_map->set_pixel(CLOUD_BITMAP, x, y, gCLOUDData[y*40+x]);
      tile_map->set_pixel(GRASS_BITMAP, x, y, gGRASSData[y*40+x]);
      tile_map->set_pixel(WALL_BITMAP, x, y, gWALLData[y*40+x]);
    }
  }*/
/*
  const char* tile_ids =
    "   C     C       C  "
    "      C             "
    "          C  C      "
    "   C               C"
    "           C        "
    "                    "
    "                    "
    "                    "
    "    WW WW  WW WW    "
    "  WWWWWWWWWWWWWWWW  "
    "  WWW    WW    WWW  "
    "  WWW    WW    WWW  "
    "..WWWWWWWWWWWWWWWW.."
    "..WWWWWWWWWWWWWWWW.."
    "...................."
    "...................."
    "...................."
    "...................."
    "...................."
    "....................";

  ;
  for (int32_t row = 0; row < 20; row++) {
    for (int32_t col = 0; col< 20; col++) {
      switch (tile_ids[row*20+col]) {
        case ' ': tile_map->set_tile(col, row, SKY_BITMAP); break;
        case 'C': tile_map->set_tile(col, row, CLOUD_BITMAP); break;
        case 'W': tile_map->set_tile(col, row, WALL_BITMAP); break;
        case '.': tile_map->set_tile(col, row, GRASS_BITMAP); break;
      }
    }
  }*/
}

void DiVideoScanLine::init_to_black() {
  memset(m_act, SYNCS_OFF, ACT_PIXELS);
  memset(m_hfp, SYNCS_OFF, HFP_PIXELS);
  memset(m_hs, (HSYNC_ON|VSYNC_OFF), HS_PIXELS);
  memset(m_hbp, SYNCS_OFF, HBP_PIXELS);
}

void DiVideoScanLine::init_for_vsync() {
  memset(m_act, (HSYNC_OFF|VSYNC_ON), ACT_PIXELS);
  memset(m_hfp, (HSYNC_OFF|VSYNC_ON), HFP_PIXELS);
  memset(m_hs, SYNCS_ON, HS_PIXELS);
  memset(m_hbp, (HSYNC_OFF|VSYNC_ON), HBP_PIXELS);
}

void show_value(DiOpaqueBitmap** digits, uint32_t y, DiPaintParams *params) {
  digits[5]->set_position(200,y);
  digits[5]->paint(params);
  digits[4]->set_position(206,y);
  digits[4]->paint(params);
  digits[3]->set_position(212,y);
  digits[3]->paint(params);
  digits[2]->set_position(218,y);
  digits[2]->paint(params);
  digits[1]->set_position(224,y);
  digits[1]->paint(params);
  digits[0]->set_position(230,y);
  digits[0]->paint(params);
}

void IRAM_ATTR DiVideoScanLine::paint(DiPaintParams *params) {
  params->m_line32 = (uint32_t*)(m_act);
  params->m_line8 = (uint8_t*)(m_act);

  memset(params->m_line8, SYNCS_OFF, ACT_PIXELS);
  //tile_map->paint(params);

  DiPaintParams p2 = *params;
  p2.m_horiz_scroll = 0;
  p2.m_vert_scroll = 0;
  p2.m_scrolled_y = p2.m_line_index;

  DiPaintParams p3 = *params;
  p3.m_horiz_scroll = 0;
  p3.m_vert_scroll = -30;
  p3.m_scrolled_y = p3.m_line_index + p3.m_vert_scroll;

  /*if (params->m_line_index >= 100 && params->m_line_index < 140) {
    show_value(gp_value_bitmap[0], 100, &p2);
    show_value(gp_value_bitmap[1], 110, &p2);
    show_value(gp_value_bitmap[2], 120, &p2);
    show_value(gp_value_bitmap[3], 130, &p2);
  }*/

  /*int32_t i = params->m_scrolled_y;
  if (i >= -(STAR_PADDING/2) && i < ACT_LINES) {
    g_stars[i+(STAR_PADDING/2)].paint(params);
  }*/

  /*g_vert_center.paint(params);
  g_horiz_center.paint(params);
  g_diamond_ne.paint(params);
  g_diamond_sw.paint(params);
  g_diamond_nw.paint(params);
  g_diamond_se.paint(params);


  // Draw small diamond shapes somewhere.
  for (int d = 0; d<10; d++) {
    uint32_t sdtop = sdy[d] - HALF_SMALL_DIAMOND_SIZE;
    uint32_t sdbottom = sdy[d] + HALF_SMALL_DIAMOND_SIZE;
    if (params->m_line_index >= sdtop && params->m_line_index <= sdbottom) {
      int sd_offset = HALF_SMALL_DIAMOND_SIZE - abs((int)(params->m_line_index - sdy[d]));
      uint8_t diamond = sdcolor[d];
      params->m_line8[FIX_INDEX(sdx[d]-1-sd_offset)] = diamond;
      params->m_line8[FIX_INDEX(sdx[d]-0-sd_offset)] = diamond;
      params->m_line8[FIX_INDEX(sdx[d]+0+sd_offset)] = diamond;
      params->m_line8[FIX_INDEX(sdx[d]+1+sd_offset)] = diamond;
    }
  }

  // Draw tiny diamond shapes somewhere.
  for (int d = 0; d<10; d++) {
    uint32_t sdtop = sdy[d] - HALF_TINY_DIAMOND_SIZE;
    uint32_t sdbottom = sdy[d] + HALF_TINY_DIAMOND_SIZE;
    if (params->m_line_index >= sdtop && params->m_line_index <= sdbottom) {
      int sd_offset = HALF_TINY_DIAMOND_SIZE - abs((int)(params->m_line_index - sdy[d]));
      uint8_t diamond = sdcolor[d];
      params->m_line8[FIX_INDEX(sdx[d]-1-sd_offset)] = diamond;
      params->m_line8[FIX_INDEX(sdx[d]-0-sd_offset)] = diamond;
      params->m_line8[FIX_INDEX(sdx[d]+0+sd_offset)] = diamond;
      params->m_line8[FIX_INDEX(sdx[d]+1+sd_offset)] = diamond;
    }
  }*/

  g_x_pixel[0].paint(&p2);
  g_x_pixel[1].paint(&p2);
  g_x_pixel[2].paint(&p2);
  g_x_pixel[3].paint(&p2);
  g_x_pixel[4].paint(&p2);

  g_y_pixel[0].paint(&p2);
  g_y_pixel[1].paint(&p2);
  g_y_pixel[2].paint(&p2);
  g_y_pixel[3].paint(&p2);
  g_y_pixel[4].paint(&p2);

  // Draw a bitmap
  for (uint32_t r = 0; r < NR; r++) {
    for (uint32_t c = 0; c < NC; c++) {
      gp_opaque_bitmap[r][c]->paint(&p3);
    }
  }

  /*gp_masked_bitmap4->paint(&p2);
  gp_masked_bitmap5->paint(&p2);
  gp_masked_bitmap6->paint(&p2);
  gp_masked_bitmap7->paint(&p2);*/

 // gp_masked_bitmap->paint(params);

  /*int32_t r = params->m_scrolled_y / 30;
  if (r >= 0 && r < TILES_DOWN) {
    for (int32_t c = 0; c < TILES_ACROSS; c++) {
      tile->set_position(c * 40, r * 30);
      tile->paint(params);
    }
  }*/
}

void DiVideoBuffer::init_to_black() {
  for (int i = 0; i < NUM_LINES_PER_BUFFER; i++) {
    m_line[i].init_to_black();
  }
}

void DiVideoBuffer::init_for_vsync() {
  for (int i = 0; i < NUM_LINES_PER_BUFFER; i++) {
    m_line[i].init_for_vsync();
  }
}

void IRAM_ATTR DiVideoBuffer::paint(DiPaintParams *params) {
  // Since there are only 2 lines per buffer, we unroll the loop here, for speed.
  m_line[0].paint(params);
  ++(params->m_line_index);
  m_line[1].paint(params);
}
