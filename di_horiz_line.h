#pragma once
#include "di_drawing_instruction.h"

class DiHorizontalLine: public DiDrawingInstrAtX {
  public:
  uint32_t m_length;
  uint8_t m_color;
  uint32_t m_color4;

  DiHorizontalLine(int32_t x, uint32_t length, uint8_t color);

  virtual void IRAM_ATTR paint(uint32_t* line, int32_t scroll);
};
