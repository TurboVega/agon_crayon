#pragma once
#include "di_drawing_instruction.h"

class DiVerticalLine: public DiDrawingInstrAtY {
  public:
  uint32_t m_length;
  uint8_t m_color;

  DiVerticalLine(int32_t y, uint32_t length, uint8_t color);

  virtual void IRAM_ATTR paint(const DiPaintParams *params);
};
