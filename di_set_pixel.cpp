#include "di_set_pixel.h"

extern "C" {
IRAM_ATTR void DiSetPixel_paint(void* this_ptr, const DiPaintParams *params);
}

DiSetPixel::DiSetPixel() {
  m_color = SYNCS_OFF;
}

DiSetPixel::DiSetPixel(int32_t x, int32_t y, uint8_t color)
  : DiDrawingInstrAtXY(x, y) {
  m_color = color | SYNCS_OFF;
}

void IRAM_ATTR DiSetPixel::paint(const DiPaintParams *params) {
  DiSetPixel_paint((void*)this, params);
  /*uint32_t result = DiSetPixel_paint(params, (uint32_t)(void*)this,
  (uint32_t)(void*)(&m_x), (uint32_t)(void*)(&m_y), (uint32_t)(void*)(&m_color));
  if (result == (uint32_t)(void*)this) {
    m_color = MASK_RGB(0,0,2) | SYNCS_OFF;
  }
  if (result+4 == (uint32_t)(void*)this) {
    m_color = MASK_RGB(0,2,0) | SYNCS_OFF;
  }
  if (result-4 == (uint32_t)(void*)this) {
    m_color = MASK_RGB(2,0,0) | SYNCS_OFF;
  }*/
  if (m_y == params->m_scrolled_index) {
    auto x = m_x;
    limit_x(x, params->m_horiz_scroll);
    if (x >= 0) {
      params->m_line8[FIX_INDEX(x)] = m_color;
    }
  }
}
