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
  uint32_t m_scroll_mode;
  uint32_t m_transparent_color;
  uint32_t m_pixels[1];

  // Construct a dynamically-sized transparent bitmap. The m_pixels array gets sized during 'new'.
  DiTransparentBitmap(uint32_t width, uint32_t height, ScrollMode scroll_mode);

  // Allocate a transparent bitmap. This takes 4x as much memory as the bitmap itself.
  void* operator new(size_t size, uint32_t width, uint32_t height, ScrollMode scroll_mode);

  // Set the position of the bitmap on the screen, and assume using pixels starting at line 0 in the bitmap.
  void set_position(int32_t x, int32_t y);

  // Set the position of the bitmap on the screen, and assume using pixels starting at the given line in the bitmap.
  // This makes it possible to use a single (tall) bitmap to support animated sprites.
  void set_position(int32_t x, int32_t y, uint32_t start_line, uint32_t height);

  // Set a single pixel within the allocated bitmap. The upper 2 bits of the color
  // are the transparency level (00BBGGRR is 25% opaque, 01BBGGRR is 50% opaque,
  // 10BBGGRR is 75% opaque, and 11BBGGRR is 100% opaque). If the given color value
  // equals the already-set transparent color, then the pixel will be fully transparent,
  // meaning 0% opaque.
  void set_transparent_pixel(int32_t x, int32_t y, uint8_t color);

  // Set the single 8-bit color value used to represent a transparent pixel. This should be
  // an unused color value in the visible image when designing the image. This does take out
  // 1 of the 256 possible color values. The upper 2 bits of the color are the transparency
  // level (00BBGGRR is 25% opaque, 01BBGGRR is 50% opaque, 10BBGGRR is 75% opaque, and
  // 11BBGGRR is 100% opaque).
  void set_transparent_color(uint8_t color);

  virtual void IRAM_ATTR paint(const DiPaintParams *params);

  protected:
  // Set a single pixel with an adjusted color value.
  void set_pixel(int32_t x, int32_t y, uint8_t color);
};
