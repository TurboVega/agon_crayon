#pragma once
#include <stdint.h>
#include <stddef.h>
#include "driver/gpio.h"
#include "di_constants.h"

typedef struct {
  uint32_t*   m_line32;
  uint8_t*    m_line8;
  int32_t     m_line_index;
  int32_t     m_scrolled_index;
  int32_t     m_horiz_scroll;
  int32_t     m_vert_scroll;
} DiPaintParams;

class DiDrawingInstruction {
  public:

  virtual void IRAM_ATTR paint(const DiPaintParams *params);

  protected:

  inline uint8_t* pixels(uint32_t* line) {
    return (uint8_t*)line;
  }

  inline void limit_x(int32_t &x, int32_t hscroll) {
    x += hscroll;
    if ((x < 0) || (x >= ACT_PIXELS)) {
      x = -1;
    }
  }

  inline void limit_y(int32_t &y, int32_t vscroll) {
    y += vscroll;
    if ((y < 0) || (y >= ACT_LINES)) {
      y = -1;
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

  inline void clamp_top(int32_t &y, int32_t &offset, int32_t vscroll) {
    y += vscroll;
    if (y >= 0) {
      offset = 0;
    } else {
      offset = -y;
      y = 0;
    }
  }

  inline void clamp_bottom(int32_t &y, int32_t vscroll) {
    y += vscroll;
    if (y >= ACT_LINES) {
      y = ACT_LINES - 1;
    }
  }
};

class DiDrawingInstrAtX: public DiDrawingInstruction {
  public:
  int32_t m_x;

  DiDrawingInstrAtX();
  DiDrawingInstrAtX(int32_t x);
};

class DiDrawingInstrAtY: public DiDrawingInstruction {
  public:
  int32_t m_y;

  DiDrawingInstrAtY();
  DiDrawingInstrAtY(int32_t y);
};

class DiDrawingInstrAtXY: public DiDrawingInstrAtX {
  public:
  int32_t m_y;

  DiDrawingInstrAtXY();
  DiDrawingInstrAtXY(int32_t x, int32_t y);
};
