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
//     [a2+28] m_bitmaps
//     [a2+32] m_columns
//     [a2+36] m_rows
//     [a2+40] m_words_per_line
//     [a2+44] m_bytes_per_line
//     [a2+48] m_words_per_bitmap
//     [a2+52] m_bytes_per_bitmap
//     [a2+56] m_words_for_bitmaps
//     [a2+60] m_bytes_for_bitmaps
//     [a2+64] m_words_per_row
//     [a2+68] m_bytes_per_row
//     [a2+72] m_words_for_tiles
//     [a2+76] m_bytes_for_tiles
//     [a2+80] m_words_for_offsets
//     [a2+84] m_bytes_for_offsets
//     [a2+88] m_tiles
//     [a2+92] m_pixels
//     [a2+96] m_offsets
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
  l32i      a4,a3,8     // a4 <-- m_line_index
  slli      a4,a4,3     // a4 <-- m_line_index * 8
  l32i      a5,a2,96    // a5 <-- m_offsets
  add       a4,a4,a5    // a4 <-- m_offsets + m_line_index * 8
  l32i      a5,a4,4     // a5 <-- m_offsets[m_line_index * 2 + 1] = offset to bitmap line
  l32i      a4,a4,0     // a4 <-- m_offsets[m_line_index * 2] =  points to tile map row
  l32i      a6,a2,32    // a6 <-- m_columns
  movi      a11,1       // a11 <-- count for decrements
  l32i      a8,a3,0     // a8 <-- m_line32
do_tiles:
  l32i      a7,a4,0     // a7 <-- points to bitmap for current tile
  add       a7,a7,a5    // a7 <-- points to bitmap line within current tile

  l32i      a10,a2,40     // a10 <-- m_words_per_line
  loop      a10,end_loop  // loop to copy pixel data
  l32i      a9,a7,0       // a9 <-- 4 src pixels
  s32i      a9,a8,0       // m_line32[] <-- src pixel data
  addi      a7,a7,4       // a7 <-- points to next src pixel
  addi      a8,a8,4       // a8 <-- points to next dst pixel
end_loop:

  addi      a4,a4,4     // a4 <-- points to next bitmap address
  sub       a6,a6,a11   // a6 <-- remaining number of columns
  bnez      a6,do_tiles // go back if more tiles to do
  retw.n
