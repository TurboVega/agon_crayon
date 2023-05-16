#pragma once
#include <stdint.h>
#include <stddef.h>
#include "driver/gpio.h"
#include "di_constants.h"

class DiDrawingInstruction {
  public:

  virtual void IRAM_ATTR paint(uint32_t* line, int32_t scroll);

  protected:

  inline uint8_t* pixels(uint32_t* line) {
    return (uint8_t*)line;
  }

  inline void limit_x(int32_t &x, int32_t scroll) {
    x += scroll;
    if ((x < 0) || (x >= ACT_PIXELS)) {
      x = -1;
    }
  }

  inline void clamp_left(int32_t &x, int32_t &offset, int32_t scroll) {
    x += scroll;
    if (x >= 0) {
      offset = 0;
    } else {
      offset = -x;
      x = 0;
    }
  }

  inline void clamp_right(int32_t &x, int32_t scroll) {
    x += scroll;
    if (x >= ACT_PIXELS) {
      x = ACT_PIXELS - 1;
    }
  }
};

class DiDrawingInstrAtX: public DiDrawingInstruction {
  public:
  int32_t m_x;

  DiDrawingInstrAtX();
  DiDrawingInstrAtX(int32_t x);
};

class DiDrawingInstrAtXY: public DiDrawingInstrAtX {
  public:
  int32_t m_y;

  DiDrawingInstrAtXY();
  DiDrawingInstrAtXY(int32_t x, int32_t y);
};
