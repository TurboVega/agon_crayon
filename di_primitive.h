// primitive.h - Function declarations for base drawing primitives
//
// A drawing primitive tells how to draw a particular type of simple graphic object.
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

typedef enum ScrollMode {
  NONE,       // do not allow scrolling
  HORIZONTAL, // allow horizontal, but not vertical
  VERTICAL,   // allow vertical, but not horizontal
  BOTH        // both horizontal and vertical
};

typedef struct {
  uint32_t* m_line32;
  uint8_t*  m_line8;
  int32_t   m_line_index;
  int32_t   m_scrolled_y;
  int32_t   m_horiz_scroll;
  int32_t   m_vert_scroll;
  int32_t   m_screen_width;
  int32_t   m_screen_height;
} DiPaintParams;

class DiPrimitive {
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

class DiPrimitiveX: public DiPrimitive {
  public:
  int32_t m_x;

  DiPrimitiveX();
  DiPrimitiveX(int32_t x);
};

class DiPrimitiveXC: public DiPrimitiveX {
  public:
  int32_t m_color;

  DiPrimitiveXC();
  DiPrimitiveXC(int32_t x, int8_t color);
};

class DiPrimitiveY: public DiPrimitive {
  public:
  int32_t m_y;

  DiPrimitiveY();
  DiPrimitiveY(int32_t y);
};

class DiPrimitiveYC: public DiPrimitiveY {
  public:
  int32_t m_color;

  DiPrimitiveYC();
  DiPrimitiveYC(int32_t y, int8_t color);
};

class DiPrimitiveXY: public DiPrimitiveX {
  public:
  int32_t m_y;

  DiPrimitiveXY();
  DiPrimitiveXY(int32_t x, int32_t y);
};

class DiPrimitiveXYC: public DiPrimitiveXY {
  public:
  int32_t m_color;

  DiPrimitiveXYC();
  DiPrimitiveXYC(int32_t x, int32_t y, int8_t color);
};

class DiPrimitiveXYW: public DiPrimitiveXY {
  public:
  int32_t m_width;
  int32_t m_x_extent;

  DiPrimitiveXYW();
  DiPrimitiveXYW(int32_t x, int32_t y, int32_t width);
};

class DiPrimitiveXYWC: public DiPrimitiveXYW {
  public:
  int32_t m_color;

  DiPrimitiveXYWC();
  DiPrimitiveXYWC(int32_t x, int32_t y, int32_t width, int8_t color);
};

class DiPrimitiveXYH: public DiPrimitiveXY {
  public:
  int32_t m_height;
  int32_t m_y_extent;

  DiPrimitiveXYH();
  DiPrimitiveXYH(int32_t x, int32_t y, int32_t height);
};

class DiPrimitiveXYHC: public DiPrimitiveXYH {
  public:
  int32_t m_color;

  DiPrimitiveXYHC();
  DiPrimitiveXYHC(int32_t x, int32_t y, int32_t height, int8_t color);
};

class DiPrimitiveXYWH: public DiPrimitiveXYW {
  public:
  int32_t m_height;
  int32_t m_y_extent;

  DiPrimitiveXYWH();
  DiPrimitiveXYWH(int32_t x, int32_t y, int32_t width, int32_t height);
};

class DiPrimitiveXYWHC: public DiPrimitiveXYWH {
  public:
  int32_t m_color;

  DiPrimitiveXYWHC();
  DiPrimitiveXYWHC(int32_t x, int32_t y, int32_t width, int32_t height, int8_t color);
};
