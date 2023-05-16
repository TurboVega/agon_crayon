#include "di_set_pixel.h"

DiSetPixel::DiSetPixel() {
  m_color = SYNCS_OFF;
}

DiSetPixel::DiSetPixel(int32_t x, int32_t y, uint8_t color)
  : DiDrawingInstrAtXY(x, y) {
  m_color = color | SYNCS_OFF;
}

void IRAM_ATTR DiSetPixel::paint(const DiPaintParams *params) {
  if (m_y == params->m_scrolled_index) {
    auto x = m_x;
    limit_x(x, params->m_horiz_scroll);
    if (x >= 0) {
      params->m_line8[FIX_INDEX(x)] = m_color;
    }
  }
}
