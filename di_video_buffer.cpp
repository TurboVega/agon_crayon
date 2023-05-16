#include <string.h>
#include "di_video_buffer.h"
#include "di_set_pixel.h"
#include "di_horiz_line.h"
#include "di_opaque_bitmap.h"

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

#define NUM_STARS 200
DiSetPixel g_stars[NUM_STARS];

DiHorizontalLine g_vert_center(300, 200, 400, MASK_RGB(3,0,0));
DiSetPixel g_horiz_center(CENTER_X, CENTER_Y, MASK_RGB(0,0,3));
DiOpaqueBitmap* gp_bitmap = new(8,8) DiOpaqueBitmap(8,8);

void init_stars() {
  srand(42);
  for (int i = 0; i < NUM_STARS; i++) {
    int32_t x = rand() % ACT_PIXELS;
    int32_t y = rand() % ACT_LINES;
    g_stars[i].m_x = x;
    g_stars[i].m_y = y;
    g_stars[i].m_color = MASK_RGB(3,3,3) | SYNCS_OFF;
  }

  gp_bitmap->set_position(700,500);
  gp_bitmap->clear();
  for (int32_t y=0;y<8;y++) {
    gp_bitmap->set_pixel(0, y, MASK_RGB(0,0,3));
    gp_bitmap->set_pixel(7, y, MASK_RGB(0,3,3));
  }
  for (int32_t x=1;x<7;x++) {
    gp_bitmap->set_pixel(x, 0, MASK_RGB(3,0,0));
    gp_bitmap->set_pixel(x, 7, MASK_RGB(3,3,0));
    gp_bitmap->set_pixel(x, x, MASK_RGB(3,0,3));
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

void IRAM_ATTR DiVideoScanLine::paint(const DiPaintParams *params) {
  memset((uint8_t*)(params->m_line8), SYNCS_OFF, ACT_PIXELS);

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
  gp_bitmap->paint(params);
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
    ++(params->m_scrolled_index);
  }
}
