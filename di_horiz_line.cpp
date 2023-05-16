#include "di_horiz_line.h"

DiHorizontalLine::DiHorizontalLine(int32_t x, uint32_t length, uint8_t color)
  : DiDrawingInstrAtX(x) {
  m_length = length;
  m_color = color | SYNCS_OFF;
  m_color4 = (((uint32_t)color) << 24) |
      (((uint32_t)color) << 16) |
      (((uint32_t)color) << 8) |
      ((uint32_t)color);
}

void IRAM_ATTR DiHorizontalLine::paint(uint32_t* line, int32_t scroll) {
  auto x = m_x;
  int32_t offset = 0;
  clamp_left(x, offset, scroll);
  int32_t x2 = m_x + m_length;
  clamp_right(x2, scroll);
  auto length = x2 - x + 1;
  auto c = m_color;
  while ((length > 0) && (x & 3)) {
    pixels(line)[FIX_INDEX(x++)] = c;
    --length;
  }
  auto index = x / 4;
  while ((length >= 4) && (index < ACT_PIXELS/4)) {
    line[index++] = m_color4;
    length -= 4;
    x += 4;
  }
  while ((length > 0) && (x < ACT_PIXELS)) {
    pixels(line)[FIX_INDEX(x++)] = c;
    --length;
  }
}
