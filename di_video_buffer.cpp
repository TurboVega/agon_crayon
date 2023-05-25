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

#define _COMPILE_HEX_DATA_
#include "TEST_BITMAP.h"

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

#define STAR_PADDING 200
#define NUM_STARS (ACT_LINES+STAR_PADDING)
DiSetPixel g_stars[NUM_STARS];

DiHorizontalLine g_vert_center(300, 300, 200, MASK_RGB(3,0,0));
DiVerticalLine g_horiz_center(CENTER_X, CENTER_Y-100, 200, MASK_RGB(0,0,3));
DiDiagonalRightLine g_diamond_ne(CENTER_X, CENTER_Y-HALF_DIAMOND_SIZE, HALF_DIAMOND_SIZE,  MASK_RGB(2,3,1));
DiDiagonalRightLine g_diamond_sw(CENTER_X-HALF_DIAMOND_SIZE, CENTER_Y, HALF_DIAMOND_SIZE,  MASK_RGB(2,3,1));
DiDiagonalLeftLine g_diamond_nw(CENTER_X, CENTER_Y-HALF_DIAMOND_SIZE, HALF_DIAMOND_SIZE,  MASK_RGB(2,3,1));
DiDiagonalLeftLine g_diamond_se(CENTER_X+HALF_DIAMOND_SIZE-1, CENTER_Y, HALF_DIAMOND_SIZE,  MASK_RGB(2,3,1));
DiOpaqueBitmap* gp_opaque_bitmap0 = new(64,64) DiOpaqueBitmap(64,64);
DiOpaqueBitmap* gp_opaque_bitmap1 = new(64,64) DiOpaqueBitmap(64,64);
DiOpaqueBitmap* gp_opaque_bitmap2 = new(64,64) DiOpaqueBitmap(64,64);
DiOpaqueBitmap* gp_opaque_bitmap3 = new(64,64) DiOpaqueBitmap(64,64);

DiOpaqueBitmap* gp_opaque_bitmap4 = new(64,64) DiOpaqueBitmap(64,64);
/*DiOpaqueBitmap* gp_opaque_bitmap5 = new(64,64) DiOpaqueBitmap(64,64);
DiOpaqueBitmap* gp_opaque_bitmap6 = new(64,64) DiOpaqueBitmap(64,64);
DiOpaqueBitmap* gp_opaque_bitmap7 = new(64,64) DiOpaqueBitmap(64,64);*/

DiMaskedBitmap* gp_masked_bitmap = new(64,64) DiMaskedBitmap(64,64);

void init_stars() {
  srand(42);
  for (int i = 0; i < NUM_STARS; i++) {
    int32_t x = rand() % (ACT_PIXELS * 3 / 2) - (ACT_PIXELS / 4);
    int8_t c = (int8_t)(rand() % 63 + 1);
    g_stars[i].m_x = x;
    g_stars[i].m_y = i-(STAR_PADDING/2);
    g_stars[i].m_color = c | SYNCS_OFF;
  }

  gp_opaque_bitmap0->set_position(270,200);
  gp_opaque_bitmap0->clear();
  gp_opaque_bitmap1->set_position(510,210);
  gp_opaque_bitmap1->clear();
  gp_opaque_bitmap2->set_position(637,455);
  gp_opaque_bitmap2->clear();
  gp_opaque_bitmap3->set_position(333,500);
  gp_opaque_bitmap3->clear();

  gp_opaque_bitmap4->set_position(170,230);
  gp_opaque_bitmap4->clear();
/*  gp_opaque_bitmap5->set_position(467,398);
  gp_opaque_bitmap5->clear();
  gp_opaque_bitmap6->set_position(339,555);
  gp_opaque_bitmap6->clear();
  gp_opaque_bitmap7->set_position(109,560);
  gp_opaque_bitmap7->clear();*/

  gp_masked_bitmap->set_position(500,200);
  gp_masked_bitmap->clear();

  for (int32_t y=0;y<64;y++) {
    for (int32_t x=0;x<64;x++) {
      gp_opaque_bitmap0->set_pixel(x, y, gtest_bitmapData[y*64+x]);
      gp_opaque_bitmap1->set_pixel(x, y, gtest_bitmapData[y*64+x]);
      gp_opaque_bitmap2->set_pixel(x, y, gtest_bitmapData[y*64+x]);
      gp_opaque_bitmap3->set_pixel(x, y, gtest_bitmapData[y*64+x]);

      gp_opaque_bitmap4->set_pixel(x, y, gtest_bitmapData[y*64+x]);
      /*gp_opaque_bitmap5->set_pixel(x, y, gtest_bitmapData[y*64+x]);
      gp_opaque_bitmap6->set_pixel(x, y, gtest_bitmapData[y*64+x]);
      gp_opaque_bitmap7->set_pixel(x, y, gtest_bitmapData[y*64+x]);*/

      gp_masked_bitmap->set_pixel(x, y, gtest_bitmapData[y*64+x]);
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

void IRAM_ATTR DiVideoScanLine::paint(DiPaintParams *params) {
  params->m_line32 = (uint32_t*)(m_act);
  params->m_line8 = (uint8_t*)(m_act);
  params->m_scrolled_y = params->m_line_index + params->m_vert_scroll;

  memset(params->m_line8, SYNCS_OFF, ACT_PIXELS);

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

/*
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
*/
  // Draw a bitmap
  DiPaintParams p2 = *params;
  p2.m_horiz_scroll = 0;
  p2.m_vert_scroll = 0;
  p2.m_scrolled_y = p2.m_line_index;

  gp_opaque_bitmap0->paint(params);
  gp_opaque_bitmap1->paint(params);
  gp_opaque_bitmap2->paint(params);
  gp_opaque_bitmap3->paint(params);

  /*gp_opaque_bitmap4->paint(params);
  gp_opaque_bitmap5->paint(params);
  gp_opaque_bitmap6->paint(params);
  gp_opaque_bitmap7->paint(params);*/

  gp_opaque_bitmap4->paint(&p2);
  //gp_opaque_bitmap5->paint(&p2);
  //gp_opaque_bitmap6->paint(&p2);
  //gp_opaque_bitmap7->paint(&p2);

  //gp_opaque_bitmap->paint(&p2);
 // gp_masked_bitmap->paint(params);
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
