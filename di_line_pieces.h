// di_line_pieces.h - Function declarations for generating line pieces
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
#include <stdint.h>

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define ABS(X) (((X) >= 0) ? (X) : (-(X)))

typedef struct {
  int16_t m_x;
  int16_t m_y;
  uint16_t m_width;
  uint16_t m_unused;
} DiLinePiece;

typedef struct {
  // destructor?

  DiLinePiece*  m_pieces;
  int16_t       m_min_x;
  int16_t       m_min_y;
  int16_t       m_max_x;
  int16_t       m_max_y;
  uint16_t      m_num_pieces;
} DiLinePieces;

void generate_line_pieces(DiLinePieces* lp, int16_t x1, int16_t y1, int16_t x2, int16_t y2);
