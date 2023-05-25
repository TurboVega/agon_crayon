// di_transparent_bitmap.cpp - Function definitions for drawing transparent bitmaps 
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

#include "di_transparent_bitmap.h"

DiTransparentBitmap::DiTransparentBitmap(uint32_t width, uint32_t height):
  DiDrawingInstrXYWH(0, 0, width, height) {
  m_words_per_line = (width + sizeof(uint32_t) - 1) / sizeof(uint32_t);
  m_bytes_per_line = m_words_per_line * sizeof(uint32_t);
}

void* DiTransparentBitmap::operator new(size_t size, uint32_t width, uint32_t height) {
  uint32_t wpl = (width + sizeof(uint32_t) - 1) / sizeof(uint32_t);
  size_t new_size = (size_t)(sizeof(DiTransparentBitmap) - sizeof(uint32_t) + (wpl * height * sizeof(uint32_t)));
  void* p = malloc(new_size);
  return p;
}

//void DiTransparentBitmap::operator delete(void*) {
//
//}

void DiTransparentBitmap::set_position(int32_t x, int32_t y) {
  m_x = x;
  m_x_extent = m_x + m_width;
  m_y = y;
  m_y_extent = m_y + m_height;
}

void DiTransparentBitmap::set_pixel(int32_t x, int32_t y, uint8_t color) { 
  pixels(m_pixels + y * m_words_per_line)[x] = (color & 0x3F) | SYNCS_OFF;
}

void DiTransparentBitmap::set_pixels(int32_t index, int32_t y, uint32_t colors) {
  m_pixels[y * m_words_per_line + index] = (colors & 0x3F3F3F3F) | SYNCS_OFF_X4;
}

void DiTransparentBitmap::clear() {
  fill(MASK_RGB(0,0,0));
}

void DiTransparentBitmap::fill(uint8_t color) {
  uint32_t color4 = (((uint32_t)color) << 24) |
      (((uint32_t)color) << 16) |
      (((uint32_t)color) << 8) |
      ((uint32_t)color) | SYNCS_OFF_X4;
  uint32_t words = m_words_per_line * m_height;
  uint32_t* dst = m_pixels;
  if (words) {
    do {
      *dst++ = color4;
    } while (--words);    
  }
}

void IRAM_ATTR DiTransparentBitmap::paint(const DiPaintParams *params) {
  if (params->m_scrolled_y >= m_y && params->m_scrolled_y < m_y + m_height) {
    auto x = m_x;
    int32_t offset = 0;
    clamp_left(x, offset, params->m_horiz_scroll);
    int32_t x2 = m_x + m_width - 1;
    clamp_right(x2, params->m_horiz_scroll);
    auto length = x2 - x + 1;
    auto row_index = params->m_scrolled_y - m_y;
    auto pix_index = offset;
    auto src = pixels(m_pixels + (row_index * m_words_per_line)) + pix_index;
    while (length > 0) {
      params->m_line8[FIX_INDEX(x++)] = *src++;
      --length;
    }
  }
}
