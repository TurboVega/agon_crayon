// di_general_line.h - Function declarations for drawing general lines
//
// A general line is 1 pixel thick, and connects any 2 points, except that
// it should not be used for vertical, horizontal, or precisely diagonal
// lines, because there are other optimized classes for those cases.
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
#include "di_primitive.h"
#include "di_line_pieces.h"

class DiGeneralLine: public DiPrimitiveXYWHC {
  public:
  DiLinePieces m_line_pieces; // determines how pixels on each scan line are written

  // This function constructs a line from two points. The upper 2 bits of
  // the color must be zeros.
  DiGeneralLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint8_t color);

  // This function constructs a solid (filled) triangle from three points. The
  // upper 2 bits of the color must be zeros.
  DiGeneralLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint8_t color);

  virtual void IRAM_ATTR paint(const DiPaintParams *params);
};
