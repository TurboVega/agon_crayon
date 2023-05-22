// di_horiz_line.cpp - Function definitions for drawing horizontal lines
//
// A horizontal line is N pixels wide and 1 pixel high.
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

#include "di_horiz_line.h"

DiHorizontalLine::DiHorizontalLine(int32_t x, int32_t y, uint32_t width, uint8_t color)
  : DiDrawingInstrXYWC(x, y, width, color) {
  m_color |= SYNCS_OFF;
  m_color4 = (((uint32_t)color) << 24) |
      (((uint32_t)color) << 16) |
      (((uint32_t)color) << 8) |
      ((uint32_t)color) | SYNCS_OFF_X4;
}

void IRAM_ATTR DiHorizontalLine::paint(const DiPaintParams *params) {
  if (m_y == params->m_scrolled_index) {
    auto x = m_x;
    int32_t offset = 0;
    clamp_left(x, offset, params->m_horiz_scroll);
    int32_t x2 = m_x + m_width;
    clamp_right(x2, params->m_horiz_scroll);
    auto width = x2 - x + 1;
    auto c = m_color;
    while ((width > 0) && (x & 3)) {
      params->m_line8[FIX_INDEX(x++)] = c;
      --width;
    }
    auto index = x / 4;
    while ((width >= 4) && (index < ACT_PIXELS/4)) {
      params->m_line32[index++] = m_color4;
      width -= 4;
      x += 4;
    }
    while ((width > 0) && (x < ACT_PIXELS)) {
      params->m_line8[FIX_INDEX(x++)] = c;
      --width;
    }
  }
}
