#pragma(once)
#include <stdint.h>
#include <stddef.h>
#include "driver/gpio.h"
#include "constants.h"

class DrawingInstruction {
  public:

  virtual void IRAM_ATTR paint(uint32_t* line, int32_t scroll);

  protected:

  inline uint8_t* pixels(uint32_t* line) {
    return (uint8_t*)line;
  }

  inline void limit_x(int32_t &x, int32_t scroll) {
    x += scroll;
    if ((x < 0) || (x >= ACT_PIXELS)) {
      x = -1;
    }
  }

  inline void clamp_left(int32_t &x, int32_t &offset, int32_t scroll) {
    x += scroll;
    if (x >= 0) {
      offset = 0;
    } else {
      offset = -x;
      x = 0;
    }
  }

  inline void clamp_right(int32_t &x, int32_t scroll) {
    x += scroll;
    if (x >= ACT_PIXELS) {
      x = ACT_PIXELS - 1;
    }
  }
};

class DrawingInstrAtX: public DrawingInstruction {
  public:
  int32_t m_x;

  DrawingInstrAtX();
  DrawingInstrAtX(int32_t x);
};

class DrawingInstrAtXY: public DrawingInstrAtX {
  public:
  int32_t m_y;

  DrawingInstrAtXY();
  DrawingInstrAtXY(int32_t x, int32_t y);
};

class SetPixel: public DrawingInstrAtXY {
  public:
  uint8_t m_color;

  SetPixel();
  SetPixel(int32_t x, int32_t y, uint8_t color);

  virtual void IRAM_ATTR paint(uint32_t* line, int32_t scroll);
};

class DrawHorizontalLine: public DrawingInstrAtX {
  public:
  uint32_t m_length;
  uint8_t m_color;
  uint32_t m_color4;

  DrawHorizontalLine(int32_t x, uint32_t length, uint8_t color);

  virtual void IRAM_ATTR paint(uint32_t* line, int32_t scroll);
};

class DrawBitmap: public DrawingInstrAtXY {
  public:
  uint32_t m_width;
  uint32_t m_height;
  uint32_t m_words_per_line;
  uint32_t m_pixels[1];

  DrawBitmap(uint32_t width, uint32_t height);
  void* operator new(size_t size, uint32_t width, uint32_t height);
  //void operator delete(void*);
  void set_position(int32_t x, int32_t y);
  void set_pixel(int32_t x, int32_t y, uint8_t color);
  void set_pixels(int32_t index, int32_t y, uint32_t colors);
  void clear();
  void fill(uint8_t color);

  virtual void IRAM_ATTR paint(uint32_t* line, int32_t line_index, int32_t scroll);
};

