// di_horiz_line.cpp - Function definitions for drawing horizontal lines
//
// A horizontal line is N pixels wide and 1 pixel high.
//
// Copyright (c) 2023 Curtis Whitley
// 
// The function defined in this source may be specified
// in a C source that calls it, using this declaration:
//
// extern "C" {
// IRAM_ATTR void DiHorizontalLine(void* this_ptr, const DiPaintParams *params);
// }
//
// Upon entry to the function, registers are set as follows:
//
// a0: return address
// a1: stack pointer
// a2: DiHorizontalLine* pointer (i.e., 'this')
//     [a2] vtable pointer
//     [a2+4] m_x
//     [a2+8] m_y
//     [a2+12] m_color
//
// a3: const DiPaintParams *params
//     [a3] m_line32
//     [a3+4] m_line8
//     [a3+8] m_line_index
//     [a3+12] m_scrolled_index
//     [a3+16] m_horiz_scroll
//     [a3+20] m_vert_scroll
//     [a3+24] m_screen_width
//     [a3+28] m_screen_height
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

  .section .iram1.text
  .align
  .global   DiHorizontalLine_paint
  .type     DiHorizontalLine_paint,@function

DiHorizontalLine_paint:
  entry     sp,16
skip:
  retw.n
