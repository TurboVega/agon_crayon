#include "DrawingInstruction.h"

void IRAM_ATTR DrawingInstruction::paint(uint32_t* line, int32_t scroll) {
}

DrawingInstrAtX::DrawingInstrAtX() {
  m_x = 0;
}

DrawingInstrAtX::DrawingInstrAtX(int32_t x) {
  m_x = x;
}

DrawingInstrAtXY::DrawingInstrAtXY() {
  m_y = 0;
}

DrawingInstrAtXY::DrawingInstrAtXY(int32_t x, int32_t y) :
  DrawingInstrAtX(x) {
  m_y = y;
}

SetPixel::SetPixel() {
  m_color = SYNCS_OFF;
}

SetPixel::SetPixel(int32_t x, int32_t y, uint8_t color)
  : DrawingInstrAtXY(x, y) {
  m_color = color | SYNCS_OFF;
}

void IRAM_ATTR SetPixel::paint(uint32_t* line, int32_t scroll) {
  auto x = m_x;
  limit_x(x, scroll);
  if (x >= 0) {
    pixels(line)[FIX_INDEX(x)] = m_color;
  }
}

DrawHorizontalLine::DrawHorizontalLine(int32_t x, uint32_t length, uint8_t color)
  : DrawingInstrAtX(x) {
  m_length = length;
  m_color = color | SYNCS_OFF;
  m_color4 = (((uint32_t)color) << 24) |
      (((uint32_t)color) << 16) |
      (((uint32_t)color) << 8) |
      ((uint32_t)color);
}

void IRAM_ATTR DrawHorizontalLine::paint(uint32_t* line, int32_t scroll) {
  auto x = m_x;
  int32_t offset = 0;
  clamp_left(x, offset, scroll);
  int32_t x2 = m_x + m_length;
  clamp_right(x2, scroll);
  auto length = x2 - x + 1;
  auto c = m_color;
  while ((length > 0) && (x & 3)) {
    pixels(line)[FIX_INDEX(x++)] = c;
    --length;
  }
  auto index = x / 4;
  while ((length >= 4) && (index < ACT_PIXELS/4)) {
    line[index++] = m_color4;
    length -= 4;
    x += 4;
  }
  while ((length > 0) && (x < ACT_PIXELS)) {
    pixels(line)[FIX_INDEX(x++)] = c;
    --length;
  }
}

DrawBitmap::DrawBitmap(uint32_t width, uint32_t height) {
  m_width = width;
  m_height = height;
  m_words_per_line = (width + sizeof(uint32_t) - 1) / sizeof(uint32_t);
}

void* DrawBitmap::operator new(size_t size, uint32_t width, uint32_t height) {
  uint32_t wpl = (width + sizeof(uint32_t) - 1) / sizeof(uint32_t);
  size_t new_size = (size_t)(sizeof(DrawBitmap) - sizeof(uint32_t) + (wpl * height * sizeof(uint32_t)));
  void* p = malloc(new_size);
  return p;
}

//void DrawBitmap::operator delete(void*) {
//
//}

void DrawBitmap::set_position(int32_t x, int32_t y) {
  m_x = x;
  m_y = y;
}

void DrawBitmap::set_pixel(int32_t x, int32_t y, uint8_t color) { 
  pixels(m_pixels + y * m_words_per_line)[x] = color | SYNCS_OFF;
}

void DrawBitmap::set_pixels(int32_t index, int32_t y, uint32_t colors) {
  m_pixels[y * m_words_per_line + index] = colors | SYNCS_OFF_X4;
}

void DrawBitmap::clear() {
  fill(MASK_RGB(0,0,0));
}

void DrawBitmap::fill(uint8_t color) {
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

void IRAM_ATTR DrawBitmap::paint(uint32_t* line, int32_t line_index, int32_t scroll) {
  auto x = m_x;
  int32_t offset = 0;
  clamp_left(x, offset, scroll);
  int32_t x2 = m_x + m_width - 1;
  clamp_right(x2, scroll);
  auto length = x2 - x + 1;
  auto index = offset;
  auto src = pixels(m_pixels + (line_index * m_words_per_line)) + index;
  while (length > 0) {
    /*if (line_index >= 0 && line_index < 8 && m_words_per_line == 2 && length>0 && length<=8 && index>=0 && index<8) {
      pixels(line)[FIX_INDEX(x++)] = MASK_RGB(0,3,0)|SYNCS_OFF;
    } else {
      pixels(line)[FIX_INDEX(x++)] = MASK_RGB(3,0,0)|SYNCS_OFF;
    }
    index++;*/
    pixels(line)[FIX_INDEX(x++)] = *src++;
    //pixels(line)[FIX_INDEX(x++)] = pixels(m_pixels+14)[index++];
    --length;
  }
}
