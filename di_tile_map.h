// di_tile_map.h - Function declarations for drawing tile maps
//
// A tile map is a set of rectangular tiles, where each tile is a bitmap of
// the same size (width and height). Tiles are arranged in a rectangular
// grid, where the entire portion of the grid that fits within the visible
// area of the screen may be displayed at any given moment. In other words
// multiple tiles show at the same time.
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

class DiTileMap: public DiPrimitiveXYWH {
  public:
  uint32_t m_bitmaps;
  uint32_t m_columns;
  uint32_t m_rows;
  uint32_t m_words_per_line;
  uint32_t m_bytes_per_line;
  uint32_t m_words_per_bitmap;
  uint32_t m_bytes_per_bitmap;
  uint32_t m_words_for_bitmaps;
  uint32_t m_bytes_for_bitmaps;
  uint32_t m_words_per_row;
  uint32_t m_bytes_per_row;
  uint32_t m_words_for_tiles;
  uint32_t m_bytes_for_tiles;
  uint32_t* m_tiles;
  uint32_t* m_pixels;

  DiTileMap(uint32_t bitmaps, uint32_t columns, uint32_t rows, uint32_t width, uint32_t height);
  //void operator delete(void*);
  void set_position(int32_t x, int32_t y);
  void set_pixel(int32_t bitmap, int32_t x, int32_t y, uint8_t color);
  void set_pixels(int32_t bitmap, int32_t index, int32_t y, uint32_t colors);
  void set_tile(int32_t column, int32_t row, int32_t bitmap);
  void clear();
  void fill(uint8_t color);

  virtual void IRAM_ATTR paint(const DiPaintParams *params);
};

