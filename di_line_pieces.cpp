// di_line_points.cpp - Function definitions for generating line points
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

#include "di_line_pieces.h"

typedef union {
  int64_t value64;
  struct {
    uint32_t low;
    int32_t high;
  } value32;
} Overlay;

void generate_line_pieces(DiLinePieces* lp, int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
  lp->m_min_x = MIN(x1, x2);
  lp->m_max_x = MAX(x1, x2);
  lp->m_min_y = MIN(y1, y2);
  lp->m_max_y = MAX(y1, y2);

  int16_t dx = lp->m_max_x - lp->m_min_x;
  int16_t dy = lp->m_max_y - lp->m_min_y;
  int16_t delta = MAX(dx, dy);

  Overlay x;
  x.value32.low = 0;
  x.value32.high = lp->m_min_x;
  int64_t delta_x = (((int64_t)(lp->m_max_x - lp->m_min_x)) << 32) / delta;

  Overlay y;
  y.value32.low = 0;
  y.value32.high = lp->m_min_y;
  int64_t delta_y = (((int64_t)(lp->m_max_y - lp->m_min_y)) << 32) / delta;

  lp->m_pieces = new DiLinePiece[delta+1];
  int32_t first_x = x.value32.high;
  int32_t first_y = y.value32.high;
  uint16_t i = 0;

  bool x_at_end = false;
  bool y_at_end = false;

  while (i < delta) {
    Overlay nx;
    Overlay ny;
    if (!x_at_end) {
      nx.value64 = x.value64 + delta_x + 0x80000000;
      if (nx.value32.high == lp->m_max_x) {
        x_at_end = true;
      }
    }
    
    if (!y_at_end) {
      ny.value64 = y.value64 + delta_y + 0x80000000;
      if (ny.value32.high == lp->m_max_y) {
        y_at_end = true;
      }
    }

    if (ny.value32.high != first_y) {
      lp->m_pieces[i].m_x = (int16_t)first_x;
      lp->m_pieces[i].m_y = (int16_t)first_y;
      uint16_t width = (uint16_t)(ABS(nx.value32.high - first_x));
      if (width == 0) {
          width = 1;
      }
      lp->m_pieces[i++].m_width = width;
      
      first_x = nx.value32.high;
      first_y = ny.value32.high;
    }

    if (x_at_end && y_at_end) {
      break;
    }

    x.value64 += delta_x;
    y.value64 += delta_y;
  }
  
  uint16_t width = (int16_t)(ABS(lp->m_max_x - first_x));
  lp->m_pieces[i].m_x = (int16_t)first_x;
  lp->m_pieces[i].m_y = (int16_t)first_y;
  if (width == 0) {
      width = 1;
  }
  lp->m_pieces[i++].m_width = width;
  lp->m_num_pieces = i;
  
  if (x1<x2 && y1>y2 || x1>x2 && y1<y2) {
    // Flip the line horizontally
    for (uint16_t j = 0; j < i; j++) {
        lp->m_pieces[j].m_x = lp->m_max_x - lp->m_pieces[j].m_x + lp->m_min_x;
    }
  }
}
