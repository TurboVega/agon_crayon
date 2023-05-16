#include "di_set_pixel.h"

DiSetPixel::DiSetPixel() {
  m_color = SYNCS_OFF;
}

DiSetPixel::DiSetPixel(int32_t x, int32_t y, uint8_t color)
  : DiDrawingInstrAtXY(x, y) {
  m_color = color | SYNCS_OFF;
}

void IRAM_ATTR DiSetPixel::paint(uint32_t* line, int32_t scroll) {
  auto x = m_x;
  limit_x(x, scroll);
  if (x >= 0) {
    pixels(line)[FIX_INDEX(x)] = m_color;
  }
}
