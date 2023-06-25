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
#include "esp_heap_caps.h"
#include <cstring>

extern "C" {
IRAM_ATTR void DiTransparentBitmap_paint(void* this_ptr, const DiPaintParams *params);
}

uint8_t* DiTransparentBitmap::m_mix_table = NULL;

DiTransparentBitmap::DiTransparentBitmap(uint32_t width, uint32_t height, ScrollMode scroll_mode):
  DiPrimitiveXYWH(0, 0, width, height) {
  switch (scroll_mode) {
    case NONE:
    case VERTICAL:
      m_words_per_line = ((width + sizeof(uint32_t) - 1) / sizeof(uint32_t)) * 2;
      m_bytes_per_line = m_words_per_line * sizeof(uint32_t);
      m_words_per_position = m_words_per_line * height;
      m_bytes_per_position = m_words_per_position * sizeof(uint32_t);
      {
        uint32_t* p = m_pixels;
        for (uint32_t i = 0; i < m_words_per_position; i+=2) {
          *p++ = 0xFFFFFFFF; // inverted mask
          *p++ = SYNCS_OFF_X4; // color
        }
      }
      break;

    case HORIZONTAL:
    case BOTH:
      m_words_per_line = ((width + sizeof(uint32_t) - 1) / sizeof(uint32_t) + 2) * 2;
      m_bytes_per_line = m_words_per_line * sizeof(uint32_t);
      m_words_per_position = m_words_per_line * height;
      m_bytes_per_position = m_words_per_position * sizeof(uint32_t);
      {
        uint32_t* p = m_pixels;
        uint32_t n = m_words_per_position * 4;
        for (uint32_t i = 0; i < n; i+=2) {
          *p++ = 0xFFFFFFFF; // inverted mask
          *p++ = SYNCS_OFF_X4; // color
        }
      }
      break;
  }
  m_mixed_colors = init_mix_table();
}

void* DiTransparentBitmap::operator new(size_t size, uint32_t width, uint32_t height, ScrollMode scroll_mode) {
  size_t new_size;
  uint32_t wpl;
  uint32_t wpp;
  uint32_t bpp;
  switch (scroll_mode) {
    case NONE:
    case VERTICAL:
      wpl = ((width + sizeof(uint32_t) - 1) / sizeof(uint32_t)) * 2;
      wpp = wpl * height;
      bpp = wpp * sizeof(uint32_t);
      new_size = (size_t)(sizeof(DiTransparentBitmap) - sizeof(uint32_t) + (bpp));
      break;

    case HORIZONTAL:
    case BOTH:
      wpl = ((width + sizeof(uint32_t) - 1) / sizeof(uint32_t) + 2) * 2;
      wpp = wpl * height;
      bpp = wpp * sizeof(uint32_t);
      new_size = (size_t)(sizeof(DiTransparentBitmap) - sizeof(uint32_t) + (bpp * 4));
      break;
  }
  void* p = heap_caps_malloc(new_size, MALLOC_CAP_32BIT|MALLOC_CAP_8BIT|MALLOC_CAP_SPIRAM);
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
  m_visible_start = m_pixels;
}

void DiTransparentBitmap::set_position(int32_t x, int32_t y, uint32_t start_line, uint32_t height) {
  m_x = x;
  m_x_extent = m_x + m_width;
  m_y = y;
  m_y_extent = m_y + height; // not m_height
  m_visible_start = m_pixels + start_line * m_words_per_line;
}

void DiTransparentBitmap::set_transparent_pixel(int32_t x, int32_t y, uint8_t color) {
  if (color & 0xC0) {
    set_pixel(x, y, (color & 0x3F) | SYNCS_OFF);
  }
}

void DiTransparentBitmap::set_pixel(int32_t x, int32_t y, uint8_t color) {
  for (uint32_t pos = 0; pos < 4; pos++) {
    uint8_t* p = pixels(m_pixels + pos * m_words_per_position + y * m_words_per_line + ((pos+x) / 4) * 2);
    int32_t index = FIX_INDEX((pos+x)&3);
    p[index] = 0x00; // inverted mask
    p[index + 4] = color;
  }
}

void IRAM_ATTR DiTransparentBitmap::paint(const DiPaintParams *params) {
  DiTransparentBitmap_paint((void*)this, params);
}

uint8_t DiTransparentBitmap::mix(uint8_t fg, uint8_t fa, uint8_t bg) {
  return (fg * fa + bg * (3 - fa)) / 3;
}

uint8_t* DiTransparentBitmap::init_mix_table() {
  if (!m_mix_table) {
    uint32_t new_size = 4 * 64 * 64;
    m_mix_table = (uint8_t*) heap_caps_malloc(new_size, MALLOC_CAP_8BIT|MALLOC_CAP_INTERNAL);
    uint8_t* p = m_mix_table;
    for (uint8_t fa = 0; fa < 4; fa++) {
      for (uint8_t fg_b = 0; fg_b < 4; fg_b++) {
        for (uint8_t fg_g = 0; fg_g < 4; fg_g++) {
          for (uint8_t fg_r = 0; fg_r < 4; fg_r++) {
            for (uint8_t bg_b = 0; bg_b < 4; bg_b++) {
              for (uint8_t bg_g = 0; bg_g < 4; bg_g++) {
                for (uint8_t bg_r = 0; bg_r < 4; bg_r++) {
                  uint8_t mb = mix(fg_b, fa, bg_b);
                  uint8_t mg = mix(fg_g, fa, bg_g);
                  uint8_t mr = mix(fg_r, fa, bg_r);
                  *p++ = ((mb<<4)|(mg<<2)|(mr)|SYNCS_OFF);
                }
              }
            }
          }
        }
      }
    }
  }
  return m_mix_table;
}