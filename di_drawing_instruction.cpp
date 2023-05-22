// di_drawing_insruction.cpp - Function definitions for base drawing instructions
//
// A drawing instruction tells how to draw a particular type of drawing primitive.
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

#include "di_drawing_instruction.h"

void IRAM_ATTR DiDrawingInstruction::paint(const DiPaintParams *params) {}

DiDrawingInstrX::DiDrawingInstrX(): m_x(0) {}

DiDrawingInstrX::DiDrawingInstrX(int32_t x): m_x(x) {}

DiDrawingInstrXC::DiDrawingInstrXC(): m_color(0) {}

DiDrawingInstrXC::DiDrawingInstrXC(int32_t x, int8_t color):
  DiDrawingInstrX(x), m_color(color) {}

DiDrawingInstrY::DiDrawingInstrY(): m_y(0) {}

DiDrawingInstrY::DiDrawingInstrY(int32_t y): m_y(y) {}

DiDrawingInstrYC::DiDrawingInstrYC(): m_color(0) {}

DiDrawingInstrYC::DiDrawingInstrYC(int32_t y, int8_t color):
  DiDrawingInstrY(y), m_color(color) {}

DiDrawingInstrXY::DiDrawingInstrXY(): m_y(0) {}

DiDrawingInstrXY::DiDrawingInstrXY(int32_t x, int32_t y):
  DiDrawingInstrX(x), m_y(y) {}

DiDrawingInstrXYC::DiDrawingInstrXYC(): m_color(0) {}

DiDrawingInstrXYC::DiDrawingInstrXYC(int32_t x, int32_t y, int8_t color):
  DiDrawingInstrXY(x, y), m_color(color) {}

DiDrawingInstrXYW::DiDrawingInstrXYW(): m_width(0) {}

DiDrawingInstrXYW::DiDrawingInstrXYW(int32_t x, int32_t y, int32_t width):
  DiDrawingInstrXY(x, y), m_width(width) {}

DiDrawingInstrXYWC::DiDrawingInstrXYWC(): m_color(0) {}

DiDrawingInstrXYWC::DiDrawingInstrXYWC(int32_t x, int32_t y, int32_t width, int8_t color):
  DiDrawingInstrXYW(x, y, width), m_color(color) {}

DiDrawingInstrXYH::DiDrawingInstrXYH(): m_height(0) {}

DiDrawingInstrXYH::DiDrawingInstrXYH(int32_t x, int32_t y, int32_t height):
  DiDrawingInstrXY(x, y), m_height(height) {}

DiDrawingInstrXYHC::DiDrawingInstrXYHC(): m_color(0) {}

DiDrawingInstrXYHC::DiDrawingInstrXYHC(int32_t x, int32_t y, int32_t height, int8_t color):
  DiDrawingInstrXYH(x, y, height), m_color(color) {}

DiDrawingInstrXYWH::DiDrawingInstrXYWH(): m_height(0) {}

DiDrawingInstrXYWH::DiDrawingInstrXYWH(int32_t x, int32_t y, int32_t width, int32_t height):
  DiDrawingInstrXYW(x, y, width), m_height(height) {}

DiDrawingInstrXYWHC::DiDrawingInstrXYWHC(): m_color(0) {}

DiDrawingInstrXYWHC::DiDrawingInstrXYWHC(int32_t x, int32_t y, int32_t width, int32_t height, int8_t color):
  DiDrawingInstrXYWH(x, y, width, height), m_color(color) {}
