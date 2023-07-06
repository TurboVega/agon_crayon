// di_general_line.cpp - Function definitions for drawing general lines
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

#include "di_general_line.h"

extern "C" {
IRAM_ATTR void DiGeneralLine_paint(void* this_ptr, const DiPaintParams *params);
}

DiGeneralLine::DiGeneralLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint8_t color)
  : DiPrimitiveXYWHC(MIN(x1,x2), MIN(y1,y2), MAX(x1,x2)-MIN(x1,x2), MAX(y1,y2)-MIN(y1,y2), color) {
  m_color =
    (((uint32_t)color) << 24) |
    (((uint32_t)color) << 16) |
    (((uint32_t)color) << 8) |
    ((uint32_t)color) | SYNCS_OFF;
  
  generate_line_pieces(&m_line_pieces, x1, y1, x2, y2);
}

void IRAM_ATTR DiGeneralLine::paint(const DiPaintParams *params) {
  DiGeneralLine_paint((void*)this, params);
}
