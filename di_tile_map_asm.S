// di_tile_map_asm.S - Low-level assembler function for drawing tile maps 
//
// A tile map is a set of rectangular tiles, where each tile is a bitmap of
// the same size (width and height). Tiles are arranged in a rectangular
// grid, where the entire portion of the grid that fits within the visible
// area of the screen may be displayed at any given moment. In other words
// multiple tiles show at the same time.
//
// Copyright (c) 2023 Curtis Whitley
// 
// The function defined in this source may be specified
// in a C source that calls it, using this declaration:
//
// extern "C" {
// IRAM_ATTR void DiTileMap_paint(void* this_ptr, const DiPaintParams *params);
// }
//
// Upon entry to the function, registers are set as follows:
//
// a0: return address
// a1: stack pointer
// a2: DiTileMap* pointer (i.e., 'this')
//     [a2] vtable pointer
//     [a2+4] m_x
//     [a2+8] m_y
//     [a2+12] m_width
//     [a2+16] m_x_extent
//     [a2+20] m_height
//     [a2+24] m_y_extent
//     [a2+28] m_words_per_line
//     [a2+32] m_bytes_per_line
//     [a2+36] m_pixels[]
//
// a3: const DiPaintParams *params
//     [a3] m_line32
//     [a3+4] m_line8
//     [a3+8] m_line_index
//     [a3+12] m_scrolled_y
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
  .global   DiTileMap_paint
  .type     DiTileMap_paint,@function

DiTileMap_paint:
  entry     sp,16
  l32i      a10,a3,4      // a10 <-- m_line8
  movi      a11,4         // a11 <-- constant for incrementing offset 
  movi      a8,40         // a8 <-- constant for incrementing pointer
  movi      a9,20         // a9 <-- number of tiles to do
  movi      a12,36        // a12 <-- offset to m_pixels array
  add       a12,a12,a2    // a12 <-- points to m_pixels array
do_tile:
  mov       a5,a12        // a5 <-- points to m_pixels array + offset
  mov       a4,a10        // a4 <-- m_line8 + offset
  movi      a6,10         // a6 <-- number of loops
  loop      a6,end_loop   // loop to copy pixel data
  l32i      a7,a5,0       // a7 <-- 4 src pixels
  s32i      a7,a4,0       // m_line8[] <-- src pixel data
  add       a5,a5,a11     // a5 <-- points to next src pixel
  add       a4,a4,a11     // a4 <-- points to next dst pixel
end_loop:
  add       a10,a10,a11   // a10 <-- m_line8 + offset
  add       a12,a12,a11   // a12 <-- points to m_pixels array + offset
  subi      a9,1          // a9 <-- tiles remaining to do
  bnz       do_tile       // back if more tiles to do
  retw.n
