#pragma once
#include "di_drawing_instruction.h"

class DiSetPixel: public DiDrawingInstrAtXY {
  public:
  uint8_t m_color;

  DiSetPixel();
  DiSetPixel(int32_t x, int32_t y, uint8_t color);

  virtual void IRAM_ATTR paint(const DiPaintParams *params);
};
