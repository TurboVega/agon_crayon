// di_masked_bitmap.h - Function declarations for drawing masked bitmaps 
//
// A masked bitmap is a combination of fully opaque and fully transparent pixels.
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

class DiMaskedBitmap: public DiPrimitiveXYWH {
  public:
  uint32_t m_words_per_line;
  uint32_t m_bytes_per_line;
  uint32_t m_pixels[1];

  DiMaskedBitmap(uint32_t width, uint32_t height);
  void* operator new(size_t size, uint32_t width, uint32_t height);
  //void operator delete(void*);
  void set_position(int32_t x, int32_t y);
  void set_pixel(int32_t x, int32_t y, uint8_t color);
  void set_pixels(int32_t index, int32_t y, uint32_t colors);
  void clear();
  void fill(uint8_t color);

  virtual void IRAM_ATTR paint(const DiPaintParams *params);
};

