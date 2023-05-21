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

void IRAM_ATTR DiDrawingInstruction::paint(const DiPaintParams *params) {
}

DiDrawingInstrAtX::DiDrawingInstrAtX() {
  m_x = 0;
}

DiDrawingInstrAtX::DiDrawingInstrAtX(int32_t x) {
  m_x = x;
}

DiDrawingInstrAtY::DiDrawingInstrAtY() {
  m_y = 0;
}

DiDrawingInstrAtY::DiDrawingInstrAtY(int32_t y) {
  m_y = y;
}

DiDrawingInstrAtXY::DiDrawingInstrAtXY() {
  m_y = 0;
}

DiDrawingInstrAtXY::DiDrawingInstrAtXY(int32_t x, int32_t y) :
  DiDrawingInstrAtX(x) {
  m_y = y;
}
