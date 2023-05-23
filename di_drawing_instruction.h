// di_drawing_insruction.h - Function declarations for base drawing instructions
//
// A drawing instruction tells how to draw a particular type of drawing primitive.
//
// Copyright (c) 2023 Curtis Whitley
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 

#pragma once
#include <stdint.h>
#include <stddef.h>
#include "driver/gpio.h"
#include "di_constants.h"

typedef struct {
  uint32_t* m_line32;
  uint8_t*  m_line8;
  int32_t   m_line_index;
  int32_t   m_scrolled_index;
  int32_t   m_horiz_scroll;
  int32_t   m_vert_scroll;
  int32_t   m_screen_width;
  int32_t   m_screen_height;
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

  inline void clamp_left(int32_t &x, int32_t &offset, int32_t hscroll) {
    x += hscroll;
    if (x >= 0) {
      offset = 0;
    } else {
      offset = -x;
      x = 0;
    }
  }

  inline void clamp_right(int32_t &x, int32_t hscroll) {
    x += hscroll;
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

class DiDrawingInstrX: public DiDrawingInstruction {
  public:
  int32_t m_x;

  DiDrawingInstrX();
  DiDrawingInstrX(int32_t x);
};

class DiDrawingInstrXC: public DiDrawingInstrX {
  public:
  int32_t m_color;

  DiDrawingInstrXC();
  DiDrawingInstrXC(int32_t x, int8_t color);
};

class DiDrawingInstrY: public DiDrawingInstruction {
  public:
  int32_t m_y;

  DiDrawingInstrY();
  DiDrawingInstrY(int32_t y);
};

class DiDrawingInstrYC: public DiDrawingInstrY {
  public:
  int32_t m_color;

  DiDrawingInstrYC();
  DiDrawingInstrYC(int32_t y, int8_t color);
};

class DiDrawingInstrXY: public DiDrawingInstrX {
  public:
  int32_t m_y;

  DiDrawingInstrXY();
  DiDrawingInstrXY(int32_t x, int32_t y);
};

class DiDrawingInstrXYC: public DiDrawingInstrXY {
  public:
  int32_t m_color;

  DiDrawingInstrXYC();
  DiDrawingInstrXYC(int32_t x, int32_t y, int8_t color);
};

class DiDrawingInstrXYW: public DiDrawingInstrXY {
  public:
  int32_t m_width;
  int32_t m_x_extent;

  DiDrawingInstrXYW();
  DiDrawingInstrXYW(int32_t x, int32_t y, int32_t width);
};

class DiDrawingInstrXYWC: public DiDrawingInstrXYW {
  public:
  int32_t m_color;

  DiDrawingInstrXYWC();
  DiDrawingInstrXYWC(int32_t x, int32_t y, int32_t width, int8_t color);
};

class DiDrawingInstrXYH: public DiDrawingInstrXY {
  public:
  int32_t m_height;
  int32_t m_y_extent;

  DiDrawingInstrXYH();
  DiDrawingInstrXYH(int32_t x, int32_t y, int32_t height);
};

class DiDrawingInstrXYHC: public DiDrawingInstrXYH {
  public:
  int32_t m_color;

  DiDrawingInstrXYHC();
  DiDrawingInstrXYHC(int32_t x, int32_t y, int32_t height, int8_t color);
};

class DiDrawingInstrXYWH: public DiDrawingInstrXYW {
  public:
  int32_t m_height;
  int32_t m_x_extent;
  int32_t m_y_extent;

  DiDrawingInstrXYWH();
  DiDrawingInstrXYWH(int32_t x, int32_t y, int32_t width, int32_t height);
};

class DiDrawingInstrXYWHC: public DiDrawingInstrXYWH {
  public:
  int32_t m_color;

  DiDrawingInstrXYWHC();
  DiDrawingInstrXYWHC(int32_t x, int32_t y, int32_t width, int32_t height, int8_t color);
};
