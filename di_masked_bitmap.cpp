// di_masked_bitmap.cpp - Function definitions for drawing masked bitmaps 
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

#include "di_masked_bitmap.h"

DiMaskedBitmap::DiMaskedBitmap(uint32_t width, uint32_t height):
  DiDrawingInstrXYWH(0, 0, width, height) {
  m_words_per_line = (width + sizeof(uint32_t) - 1) / sizeof(uint32_t);
}

void* DiMaskedBitmap::operator new(size_t size, uint32_t width, uint32_t height) {
  uint32_t wpl = (width + sizeof(uint32_t) - 1) / sizeof(uint32_t);
  size_t new_size = (size_t)(sizeof(DiMaskedBitmap) - sizeof(uint32_t) + (wpl * height * sizeof(uint32_t)));
  void* p = malloc(new_size);
  return p;
}

//void DiMaskedBitmap::operator delete(void*) {
//
//}

void DiMaskedBitmap::set_position(int32_t x, int32_t y) {
  m_x = x;
  m_y = y;
}

void DiMaskedBitmap::set_pixel(int32_t x, int32_t y, uint8_t color) { 
  pixels(m_pixels + y * m_words_per_line)[x] = color;
}

void DiMaskedBitmap::set_pixels(int32_t index, int32_t y, uint32_t colors) {
  m_pixels[y * m_words_per_line + index] = colors;
}

void DiMaskedBitmap::clear() {
  fill(MASK_RGB(0,0,0));
}

void DiMaskedBitmap::fill(uint8_t color) {
  uint32_t color4 = (((uint32_t)color) << 24) |
      (((uint32_t)color) << 16) |
      (((uint32_t)color) << 8) |
      ((uint32_t)color);
  uint32_t words = m_words_per_line * m_height;
  uint32_t* dst = m_pixels;
  if (words) {
    do {
      *dst++ = color4;
    } while (--words);    
  }
}

void IRAM_ATTR DiMaskedBitmap::paint(const DiPaintParams *params) {
  if (params->m_scrolled_index >= m_y && params->m_scrolled_index < m_y + m_height) {
    auto x = m_x;
    int32_t offset = 0;
    clamp_left(x, offset, params->m_horiz_scroll);
    int32_t x2 = m_x + m_width - 1;
    clamp_right(x2, params->m_horiz_scroll);
    auto length = x2 - x + 1;
    auto row_index = params->m_scrolled_index - m_y;
    auto pix_index = offset;
    auto src = pixels(m_pixels + (row_index * m_words_per_line)) + pix_index;
    while (length > 0) {
      uint8_t color = *src++;
      if (color & 0xC0) {
        params->m_line8[FIX_INDEX(x++)] = (color & 0x3F) | SYNCS_OFF;
      } else {
        ++x;
      }
      --length;
    }
  }
}

