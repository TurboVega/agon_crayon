#pragma once
#include "di_drawing_instruction.h"

class DiOpaqueBitmap: public DiDrawingInstrAtXY {
  public:
  uint32_t m_width;
  uint32_t m_height;
  uint32_t m_words_per_line;
  uint32_t m_pixels[1];

  DiOpaqueBitmap(uint32_t width, uint32_t height);
  void* operator new(size_t size, uint32_t width, uint32_t height);
  //void operator delete(void*);
  void set_position(int32_t x, int32_t y);
  void set_pixel(int32_t x, int32_t y, uint8_t color);
  void set_pixels(int32_t index, int32_t y, uint32_t colors);
  void clear();
  void fill(uint8_t color);

  virtual void IRAM_ATTR paint(uint32_t* line, int32_t line_index, int32_t scroll);
};

