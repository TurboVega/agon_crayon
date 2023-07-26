// di_primitive_group.h - Function declarations for grouping primitives
//
// A primitive group is a group of display primtives that can be shown,
// hidden, or moved as a unit (as a collection).
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
#include <vector>

class DiPrimitiveGroup: public DiPrimitive {
  public:
  DiPrimitiveGroup();
  virtual ~DiPrimitiveGroup();
  virtual void get_vertical_line_range(int32_t* min_y, int32_t* max_y);
  virtual void IRAM_ATTR paint(const DiPaintParams *params);

  void add_primitive(DiPrimitive* prim);
  void remove_primitive(DiPrimitive* prim);
  void set_offsets(int32_t delta_x, int32_t delta_y);

  protected:
  int32_t m_delta_x;
  int32_t m_delta_y;
  std::vector<DiPrimitive*> m_children;
};
