#include "di_opaque_bitmap.h"

DiOpaqueBitmap::DiOpaqueBitmap(uint32_t width, uint32_t height) {
  m_width = width;
  m_height = height;
  m_words_per_line = (width + sizeof(uint32_t) - 1) / sizeof(uint32_t);
}

void* DiOpaqueBitmap::operator new(size_t size, uint32_t width, uint32_t height) {
  uint32_t wpl = (width + sizeof(uint32_t) - 1) / sizeof(uint32_t);
  size_t new_size = (size_t)(sizeof(DiOpaqueBitmap) - sizeof(uint32_t) + (wpl * height * sizeof(uint32_t)));
  void* p = malloc(new_size);
  return p;
}

//void DiOpaqueBitmap::operator delete(void*) {
//
//}

void DiOpaqueBitmap::set_position(int32_t x, int32_t y) {
  m_x = x;
  m_y = y;
}

void DiOpaqueBitmap::set_pixel(int32_t x, int32_t y, uint8_t color) { 
  pixels(m_pixels + y * m_words_per_line)[x] = color | SYNCS_OFF;
}

void DiOpaqueBitmap::set_pixels(int32_t index, int32_t y, uint32_t colors) {
  m_pixels[y * m_words_per_line + index] = colors | SYNCS_OFF_X4;
}

void DiOpaqueBitmap::clear() {
  fill(MASK_RGB(0,0,0));
}

void DiOpaqueBitmap::fill(uint8_t color) {
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

void IRAM_ATTR DiOpaqueBitmap::paint(const DiPaintParams *params) {
  if (params->m_line_index >= m_y && params->m_line_index < m_y + m_length) {
    auto x = m_x;
    int32_t offset = 0;
    clamp_left(x, offset, params->m_horiz_scroll);
    int32_t x2 = m_x + m_width - 1;
    clamp_right(x2, params->m_horiz_scroll);
    auto length = x2 - x + 1;
    auto pix_index = offset;
    auto src = pixels(m_pixels + (params->m_line_index * m_words_per_line)) + pix_index;
    while (length > 0) {
      params->line8[FIX_INDEX(x++)] = *src++;
      --length;
    }
  }
}
