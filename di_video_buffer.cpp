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
#include "di_opaque_bitmap.h"
#include "di_masked_bitmap.h"
#include "di_tile_map.h"
#include "esp_heap_caps.h"

#define _COMPILE_HEX_DATA_
#define __root /**/
#include "samples\\SKY.h"
#include "samples\\CLOUD.h"
#include "samples\\GRASS.h"
#include "samples\\WALL.h"

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
/*DiOpaqueBitmap* gp_opaque_bitmap0;
DiOpaqueBitmap* gp_opaque_bitmap1;
DiOpaqueBitmap* gp_opaque_bitmap2;
DiOpaqueBitmap* gp_opaque_bitmap3;

DiMaskedBitmap* gp_masked_bitmap4;
DiMaskedBitmap* gp_masked_bitmap5;
DiMaskedBitmap* gp_masked_bitmap6;
DiMaskedBitmap* gp_masked_bitmap7;

DiOpaqueBitmap* gp_digit_bitmap[10];
DiOpaqueBitmap* gp_value_bitmap[4][6];

//DiMaskedBitmap* gp_masked_bitmap = new(64,64) DiMaskedBitmap(64,64);

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

DiTileMap* sky;
DiTileMap* cloud;
DiTileMap* grass;
DiTileMap* wall;

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
  }

  for (uint32_t d = 0; d < 10; d++) {
    gp_digit_bitmap[d] = new(5,7) DiOpaqueBitmap(5,7);
    for (int32_t y=0;y<7;y++) {
      for (int32_t x=0;x<5;x++) {
        gp_digit_bitmap[d]->set_pixel(x, y, (uint8_t)digit_data[d*(5*7) + y*5 +x]);
      }
    }
  }

  gp_opaque_bitmap0 = new(64,64) DiOpaqueBitmap(64,64);
  gp_opaque_bitmap1 = new(64,64) DiOpaqueBitmap(64,64);
  gp_opaque_bitmap2 = new(64,64) DiOpaqueBitmap(64,64);
  gp_opaque_bitmap3 = new(64,64) DiOpaqueBitmap(64,64);

  gp_masked_bitmap4 = new(64,64) DiMaskedBitmap(64,64);
  gp_masked_bitmap5 = new(64,64) DiMaskedBitmap(64,64);
  gp_masked_bitmap6 = new(64,64) DiMaskedBitmap(64,64);
  gp_masked_bitmap7 = new(64,64) DiMaskedBitmap(64,64);

  gp_opaque_bitmap0->set_position(100,100);
  gp_opaque_bitmap0->clear();
  gp_opaque_bitmap1->set_position(201,201);
  gp_opaque_bitmap1->clear();
  gp_opaque_bitmap2->set_position(302,302);
  gp_opaque_bitmap2->clear();
  gp_opaque_bitmap3->set_position(403,403);
  gp_opaque_bitmap3->clear();

  gp_masked_bitmap4->set_position(400,100);
  gp_masked_bitmap4->clear();
  gp_masked_bitmap5->set_position(501,201);
  gp_masked_bitmap5->clear();
  gp_masked_bitmap6->set_position(603,303);
  gp_masked_bitmap6->clear();
  gp_masked_bitmap7->set_position(703,403);
  gp_masked_bitmap7->clear();

  //gp_masked_bitmap->set_position(500,200);
  //gp_masked_bitmap->clear();

  for (int32_t y=0;y<64;y++) {
    for (int32_t x=0;x<64;x++) {
      gp_opaque_bitmap0->set_pixel(x, y, gtest_bitmapData[y*64+x]);
      gp_opaque_bitmap1->set_pixel(x, y, gtest_bitmapData[y*64+x]);
      gp_opaque_bitmap2->set_pixel(x, y, gtest_bitmapData[y*64+x]);
      gp_opaque_bitmap3->set_pixel(x, y, gtest_bitmapData[y*64+x]);

      gp_masked_bitmap4->set_pixel(x, y, gtest_bitmapData[y*64+x]);
      gp_masked_bitmap5->set_pixel(x, y, gtest_bitmapData[y*64+x]);
      gp_masked_bitmap6->set_pixel(x, y, gtest_bitmapData[y*64+x]);
      gp_masked_bitmap7->set_pixel(x, y, gtest_bitmapData[y*64+x]);

      //gp_masked_bitmap->set_pixel(x, y, gtest_bitmapData[y*64+x]);
    }
  }

  breakdown_value(heap_caps_get_free_size(MALLOC_CAP_32BIT|MALLOC_CAP_8BIT|MALLOC_CAP_INTERNAL), gp_value_bitmap[0]);
  breakdown_value(heap_caps_get_largest_free_block(MALLOC_CAP_32BIT|MALLOC_CAP_8BIT|MALLOC_CAP_INTERNAL), gp_value_bitmap[1]);
  breakdown_value(heap_caps_get_free_size(MALLOC_CAP_INTERNAL), gp_value_bitmap[2]);
  breakdown_value(heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL), gp_value_bitmap[3]);
*/

  /*tile = new(40,30) DiOpaqueBitmap(40,30);
  tile->clear();
  for (int32_t y=0;y<30;y++) {
    for (int32_t x=0;x<40;x++) {
      tile->set_pixel(x, y, gtest_bitmapData[y*64+x]);
    }
  }*/

  sky = new(40,30) DiTileMap(40,30);
  sky->clear();
  for (int32_t y=0;y<30;y++) {
    for (int32_t x=0;x<40;x++) {
      sky->set_pixel(x, y, gSKYData[y*64+x]);
    }
  }

  cloud = new(40,30) DiTileMap(40,30);
  cloud->clear();
  for (int32_t y=0;y<30;y++) {
    for (int32_t x=0;x<40;x++) {
      cloud->set_pixel(x, y, gCLOUDData[y*64+x]);
    }
  }

  grass = new(40,30) DiTileMap(40,30);
  grass->clear();
  for (int32_t y=0;y<30;y++) {
    for (int32_t x=0;x<40;x++) {
      grass->set_pixel(x, y, gGRASSData[y*64+x]);
    }
  }

  wall = new(40,30) DiTileMap(40,30);
  wall->clear();
  for (int32_t y=0;y<30;y++) {
    for (int32_t x=0;x<40;x++) {
      wall->set_pixel(x, y, gWALLData[y*64+x]);
    }
  }
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

/*void show_value(DiOpaqueBitmap** digits, uint32_t y, DiPaintParams *params) {
  digits[5]->set_position(700,y);
  digits[5]->paint(params);
  digits[4]->set_position(706,y);
  digits[4]->paint(params);
  digits[3]->set_position(712,y);
  digits[3]->paint(params);
  digits[2]->set_position(718,y);
  digits[2]->paint(params);
  digits[1]->set_position(724,y);
  digits[1]->paint(params);
  digits[0]->set_position(730,y);
  digits[0]->paint(params);
}*/

void IRAM_ATTR DiVideoScanLine::paint(DiPaintParams *params) {
  params->m_line32 = (uint32_t*)(m_act);
  params->m_line8 = (uint8_t*)(m_act);
  params->m_scrolled_y = params->m_line_index + params->m_vert_scroll;

//  memset(params->m_line8, SYNCS_OFF, ACT_PIXELS);
/*
  if (params->m_scrolled_y >= 500) {
    show_value(gp_value_bitmap[0], 500, params);
    show_value(gp_value_bitmap[1], 510, params);
    show_value(gp_value_bitmap[2], 520, params);
    show_value(gp_value_bitmap[3], 530, params);
  }

  int32_t i = params->m_scrolled_y;
  if (i >= -(STAR_PADDING/2) && i < ACT_LINES) {
    g_stars[i+(STAR_PADDING/2)].paint(params);
  }

  g_vert_center.paint(params);
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
  }

  // Draw a bitmap
  DiPaintParams p2 = *params;
  p2.m_horiz_scroll = 0;
  p2.m_vert_scroll = 0;
  p2.m_scrolled_y = p2.m_line_index;

  gp_opaque_bitmap0->paint(params);
  gp_opaque_bitmap1->paint(params);
  gp_opaque_bitmap2->paint(params);
  gp_opaque_bitmap3->paint(params);

  gp_masked_bitmap4->paint(&p2);
  gp_masked_bitmap5->paint(&p2);
  gp_masked_bitmap6->paint(&p2);
  gp_masked_bitmap7->paint(&p2);

  //gp_opaque_bitmap->paint(&p2);
 // gp_masked_bitmap->paint(params);
 */

  /*int32_t r = params->m_scrolled_y / 30;
  if (r >= 0 && r < TILES_DOWN) {
    for (int32_t c = 0; c < TILES_ACROSS; c++) {
      tile->set_position(c * 40, r * 30);
      tile->paint(params);
    }
  }*/

  if (params->m_line_index < 100) {
    sky->paint(params);
  } else if (params->m_line_index < 200) {
    cloud->paint(params);
  } else if (params->m_line_index < 300) {
    sky->paint(params);
  } else if (params->m_line_index < 400) {
    wall->paint(params);    
  } else {
    grass->paint(params);
  }
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
