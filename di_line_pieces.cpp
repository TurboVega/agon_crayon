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

#include "di_line_points.h"

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define ABS(X) (((X) >= 0) ? (X) : (-(X)))

typedef union {
  uint32_t value32;
  struct {
    uint16_t low;
    uint16_t high;
  } value16;
} Overlay;

LinePieces* generate_line_pieces(int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
  LinePieces* lp = new LinePieces;
  lp->m_min_x = MIN(x1, x2);
  lp->m_max_x = MAX(x1, x2);
  lp->m_min_y = MIN(y1, y2);
  lp->m_max_y = MAX(y1, y2);

  int16_t dx = max_x - min_x;
  int16_t dy = max_y - min_y;
  int16_t delta = MAX(dx, dy);

  Overlay x;
  int16_t x_end;
  int32_t delta_x;
  int32_t delta_y;

  if (y1 < y2) {
    delta_x = (((uint32_t)(x2 - x1)) << 16) / delta;
    x = x1;
    x_end = x2;
  } else {
    delta_x = (((uint32_t)(x1 - x2)) << 16) / delta;
    x = x2;
    x_end = x1;
  }

  delta_y = (((uint32_t)(max_y - m_min_y)) << 16) / delta;
  int16_t y = lp->m_min_y;

  lp->m_pieces = new LinePiece[delta];
  uint16_t first_x = x;
  uint16_t first_y = y;
  uint16_t last_x = x;
  uint16_t i = 0;

  while (true) {
    int16_t x += delta_x;
    int16_t y += delta_y;
    if (y != first_y) {
      lp->m_pieces[i].m_x = MIN(first_x, last_x);
      lp->m_pieces[i].m_y = first_y;
      lp->m_pieces[i++].m_width = ABS(last_x - first_x) + 1;
      if (x == x_end && y == lp->m_max_y) {
        break;
      }
      first_x = x;
      first_y = y;
    }
    last_x = x;
    last_y = y;
  }
  lp->m_num_pieces = i;
}
