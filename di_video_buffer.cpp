#include <string.h>
#include "di_video_buffer.h"
#include "di_set_pixel.h"
#include "di_horiz_line.h"
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

#define NUM_STARS 100
DiSetPixel g_stars[NUM_STARS];

DiHorizontalLine g_vert_center(300, 300, 200, MASK_RGB(3,0,0));
DiSetPixel g_horiz_center(CENTER_X, CENTER_Y, MASK_RGB(0,0,3));
DiOpaqueBitmap* gp_opaque_bitmap = new(64,64) DiOpaqueBitmap(64,64);
DiMaskedBitmap* gp_masked_bitmap = new(64,64) DiMaskedBitmap(64,64);

void init_stars() {
  srand(42);
  for (int i = 0; i < NUM_STARS; i++) {
    int32_t x = rand() % ACT_PIXELS;
    int32_t y = rand() % ACT_LINES;
    g_stars[i].m_x = x;
    g_stars[i].m_y = y;
    g_stars[i].m_color = MASK_RGB(3,3,3) | SYNCS_OFF;
  }

  gp_opaque_bitmap->set_position(270,200);
  gp_opaque_bitmap->clear();

  gp_masked_bitmap->set_position(500,200);
  gp_masked_bitmap->clear();

  for (int32_t y=0;y<64;y++) {
    for (int32_t x=0;x<64;x++) {
      gp_opaque_bitmap->set_pixel(x, y, gtest_bitmapData[y*64+x]);
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
  params->m_scrolled_index = params->m_line_index + params->m_vert_scroll;

  memset(params->m_line8, SYNCS_OFF, ACT_PIXELS);

  for (int i = 0; i < NUM_STARS; i++) {
    g_stars[i].paint(params);
  }

  g_vert_center.paint(params);
  g_horiz_center.paint(params);

  // Draw a large diamond shape in the center of the screen.
  auto y = params->m_scrolled_index;
  if (y >= DIAMOND_START_LINE && y <= DIAMOND_END_LINE) {
    int center_offset = HALF_DIAMOND_SIZE - abs((int)(y - CENTER_Y));
    uint8_t diamond = SYNCS_OFF | MASK_RGB(2,3,1);
    params->m_line8[FIX_INDEX(CENTER_X-1-center_offset+params->m_horiz_scroll)] = diamond;
    params->m_line8[FIX_INDEX(CENTER_X-0-center_offset+params->m_horiz_scroll)] = diamond;
    params->m_line8[FIX_INDEX(CENTER_X+0+center_offset+params->m_horiz_scroll)] = diamond;
    params->m_line8[FIX_INDEX(CENTER_X+1+center_offset+params->m_horiz_scroll)] = diamond;
  }
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
  gp_opaque_bitmap->paint(params);
  gp_masked_bitmap->paint(params);
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
  for (int i = 0; i < NUM_LINES_PER_BUFFER; i++) {
    m_line[i].paint(params);
    ++(params->m_line_index);
  }
}
