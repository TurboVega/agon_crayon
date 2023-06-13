// di_bitmap.h - Function declarations for drawing rectangular bitmaps 
//
// An opaque bitmap is a rectangle of fully opaque pixels of various colors.
//
// A masked bitmap is a combination of fully opaque of various colors,and fully
// transparent pixels.
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

class DiOpaqueBitmap: public DiPrimitiveXYWH {
  public:
  uint32_t m_words_per_line;
  uint32_t m_bytes_per_line;
  uint32_t m_words_per_position;
  uint32_t m_bytes_per_position;
  uint32_t m_pixels[1];

  DiOpaqueBitmap(uint32_t width, uint32_t height);
  void* operator new(size_t size, uint32_t width, uint32_t height);
  //void operator delete(void*);
  void set_position(int32_t x, int32_t y);
  void set_opaque_pixel(int32_t x, int32_t y, uint8_t color);

  virtual void IRAM_ATTR paint(const DiPaintParams *params);

  protected:
  void set_pixel(int32_t x, int32_t y, uint8_t color);
};

//---------------------------------------------------------------------

class DiMaskedBitmap: public DiOpaqueBitmap {
  public:
  DiMaskedBitmap(uint32_t width, uint32_t height);
  void* operator new(size_t size, uint32_t width, uint32_t height);
  void set_masked_pixel(int32_t x, int32_t y, uint8_t color);

  virtual void IRAM_ATTR paint(const DiPaintParams *params);
};

//---------------------------------------------------------------------

class DiTransparentBitmap: public DiOpaqueBitmap {
  public:
  DiTransparentBitmap(uint32_t width, uint32_t height);
  void* operator new(size_t size, uint32_t width, uint32_t height);
  void set_transparent_pixel(int32_t x, int32_t y, uint8_t color);

  virtual void IRAM_ATTR paint(const DiPaintParams *params);
};

