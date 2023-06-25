// di_bitmap.h - Function declarations for drawing transparent bitmaps 
//
// An transparent bitmap is a rectangle that is a combination of fully transparent pixels,
// partially transparent pixels, and fully opaque pixels, of various colors. 
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

class DiTransparentBitmap: public DiPrimitiveXYWH {
  public:
  uint32_t m_words_per_line;
  uint32_t m_bytes_per_line;
  uint32_t m_words_per_position;
  uint32_t m_bytes_per_position;
  uint32_t* m_visible_start;
  uint8_t* m_mixed_colors;
  uint32_t m_pixels[1];
  static uint8_t* m_mix_table;

  DiTransparentBitmap(uint32_t width, uint32_t height, ScrollMode scroll_mode);
  void* operator new(size_t size, uint32_t width, uint32_t height, ScrollMode scroll_mode);
  //void operator delete(void*);
  void set_position(int32_t x, int32_t y);
  void set_position(int32_t x, int32_t y, uint32_t start_line, uint32_t height);
  void set_transparent_pixel(int32_t x, int32_t y, uint8_t color);

  virtual void IRAM_ATTR paint(const DiPaintParams *params);

  protected:
  void set_pixel(int32_t x, int32_t y, uint8_t color);
  uint8_t mix(uint8_t fg, uint8_t fa, uint8_t bg);
  uint8_t* init_mix_table();
};
