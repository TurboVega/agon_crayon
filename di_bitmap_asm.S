// di_bitmap_asm.S - Low-level assembler function for drawing opaque bitmaps 
//
// An opaque bitmap is a rectangle of fully opaque pixels of various colors.
//
// A masked bitmap is a combination of fully opaque of various colors,and fully
// transparent pixels.
//
// An transparent bitmap is a rectangle that is a combination of fully transparent pixels,
// partially transparent pixels, and fully opaque pixels, of various colors. 
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
//
// The functions defined in this source may be specified
// in a C source that calls it, using this declaration:
//
// extern "C" {
// IRAM_ATTR void DiOpaqueBitmap_paint(void* this_ptr, const DiPaintParams *params);
// IRAM_ATTR void DiMaskedBitmap_paint(void* this_ptr, const DiPaintParams *params);
// IRAM_ATTR void DiTransparentBitmap_paint(void* this_ptr, const DiPaintParams *params);
// }
//
// Upon entry to the function, registers are set as follows:
//
// a0: return address
// a1: stack pointer
// a2: DiOpaqueBitmap* pointer (i.e., 'this')
//     [a2] vtable pointer
//     [a2+4] m_x
//     [a2+8] m_y
//     [a2+12] m_width
//     [a2+16] m_x_extent
//     [a2+20] m_height
//     [a2+24] m_y_extent
//     [a2+28] m_words_per_line
//     [a2+32] m_bytes_per_line
//     [a2+36] m_words_per_position;
//     [a2+40] m_bytes_per_position;
//     [a2+44] m_pixels[]
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
  .global   DiOpaqueBitmap_paint
  .type     DiOpaqueBitmap_paint,@function
  .global   DiMaskedBitmap_paint
  .type     DiMaskedBitmap_paint,@function


  .align    4
DiOpaqueBitmap_paint:
  entry     sp,16
  l32i      a4,a3,12      // a4 <-- m_scrolled_y
  l32i      a5,a2,8       // a5 <-- m_y
  blt       a4,a5,skip    // skip if scan line is above bitmap area
  l32i      a6,a2,24      // a6 <-- m_y_extent
  bge       a4,a6,skip    // skip if scan line is below bitmap area
  sub       a12,a4,a5     // a12 <-- m_scrolled_y - m_y
  l32i      a4,a3,4       // a4 <-- m_line8

  l32i      a5,a2,4       // a5 <-- m_x
  l32i      a6,a3,16      // a6 <-- m_horiz_scroll
  add       a5,a5,a6      // a5 (left_hand_x) <-- m_x + m_horiz_scroll
  mov       a13,a5        // a13 (left_hand_x) <-- m_x + m_horiz_scroll
  l32i      a7,a3,24      // a7 <-- m_screen_width
  bge       a5,a7,skip    // skip if left_hand_x >= m_screen_width
  bgez      a5,left_ok    // don't shorten line if left_hand_x >= 0
  xor       a5,a5,a5      // a5 (left_hand_x) <-- 0
left_ok:
  sub       a13,a5,a13    // a13 <-- offset to first visible pixel
  l32i      a8,a2,16      // a8 <-- m_x_extent  
  add       a8,a8,a6      // a8 (right_hand_x) <-- m_x_extent + m_horiz_scroll
  beqz      a8,skip       // skip if right_hand_x == 0
  bltz      a8,skip       // skip if right_hand_x < 0
  blt       a8,a7,right_ok  // don't shorten line if right_hand_x < m_screen_width
  mov       a8,a7         // a8 (right_hand_x) <-- m_screen_width
right_ok:

  sub       a3,a8,a5      // a3 <-- final line width
  beqz      a3,skip       // skip if nothing to draw
  bltz      a3,skip       // skip if nothing to draw
  l32i      a14,a2,32     // a14 <-- m_bytes_per_line
  mul16u    a14,a14,a12   // a14 <-- m_bytes_per_line * (m_scrolled_y - m_y)
  addi      a14,a14,44    // a14 <-- offset within m_pixels array
  add       a14,a14,a2    // a14 <-- pointer to bitmap data line
  add       a14,a14,a13   // a14 <-- pointer to next bitmap pixel
  movi      a6,3          // a6 <-- mask for checking byte alignment
  movi      a8,2          // value for adjusting index
  movi      a10,1         // used to decrement width by 1
  movi      a11,4         // used to decrement width by 4

// At this point:
// a3 <-- final line width
// a4 <-- m_line8 (or m_line32)
// a5 <-- final start x (destination)
// a14 <-- pointer to bitmap pixel (source)

align_left_edge:
  beqz      a3,skip       // skip if no pixels left to draw
  and       a7,a14,a6     // a7 <-- (ptr to bitmap pixel) & 3
  bnez      a7,unaligned  // go if src pixel address is not aligned
  and       a7,a5,a6      // a7 <-- x & 3
  beqz      a7,left_aligned // don't align if already aligned
unaligned:
  xor       a9,a5,a8      // a9 <-- x ^ 2
  add       a9,a9,a4      // a9 <-- m_line8 + x
  l8ui      a2,a14,0      // a2 <-- pixel from bitmap
  s8i       a2,a9,0       // m_line8[index] <-- m_color
  add       a14,a14,a10   // a14 <-- pointer to next bitmap pixel
  sub       a3,a3,a10     // a3 <-- remaining line width
  add       a5,a5,a10     // a5 <-- next x position
  bgez      a5,align_left_edge // go check alignment
left_aligned:

  blt       a3,a11,middle_done // go if no full words to set
do_middle:
  add       a9,a5,a4      // a9 <-- m_line8 + x
  l32i      a2,a14,0      // a2 <-- 4 pixels from bitmap
  slli      a7,a2,16      // a7 <-- lower-half-word in upper-half-word position
  srli      a2,a2,16      // a2 <-- upper-half-word in lower-half-word position
  or        a2,a2,a7      // a2 <-- half-words swapped
  s32i      a2,a9,0       // m_line32[index] <-- m_color (4 pixels)
  add       a14,a14,a11   // a14 <-- pointer to next bitmap pixel
  sub       a3,a3,a11     // a3 <-- remaining line width
  add       a5,a5,a11     // a5 <-- next x position
  bge       a3,a11,do_middle // go if more full words to set
middle_done:

right_edge:
  beqz      a3,skip       // skip if no pixels left to draw
  xor       a9,a5,a8      // a9 <-- x ^ 2
  add       a9,a9,a4      // a9 <-- m_line8 + x
  l8ui      a2,a14,0      // a2 <-- pixel from bitmap
  s8i       a2,a9,0       // m_line8[index] <-- m_color
  add       a14,a14,a10   // a14 <-- points to next bitmap pixel
  sub       a3,a3,a10     // a3 <-- remaining line width
  add       a5,a5,a10     // a5 <-- next x position
  bgez      a5,right_edge // go finish drawing

skip:
  retw

//----------------------------------------------------------------

  .align    4
DiMaskedBitmap_paint:
  entry     sp,16
  l32i      a4,a3,12      // a4 <-- m_scrolled_y
  l32i      a5,a2,8       // a5 <-- m_y
  blt       a4,a5,skip_m  // skip if scan line is above bitmap area
  l32i      a6,a2,24      // a6 <-- m_y_extent
  bge       a4,a6,skip_m  // skip if scan line is below bitmap area
  sub       a12,a4,a5     // a12 <-- m_scrolled_y - m_y
  l32i      a4,a3,4       // a4 <-- m_line8

  l32i      a5,a2,4       // a5 <-- m_x
  l32i      a6,a3,16      // a6 <-- m_horiz_scroll
  add       a5,a5,a6      // a5 (left_hand_x) <-- m_x + m_horiz_scroll
  mov       a13,a5        // a13 (left_hand_x) <-- m_x + m_horiz_scroll
  l32i      a7,a3,24      // a7 <-- m_screen_width
  bge       a5,a7,skip_m  // skip if left_hand_x >= m_screen_width
  bgez      a5,left_ok_m  // don't shorten line if left_hand_x >= 0
  xor       a5,a5,a5      // a5 (left_hand_x) <-- 0
left_ok_m:
  sub       a13,a5,a13    // a13 <-- offset to first visible pixel
  l32i      a8,a2,16      // a8 <-- m_x_extent  
  add       a8,a8,a6      // a8 (right_hand_x) <-- m_x_extent + m_horiz_scroll
  beqz      a8,skip       // skip if right_hand_x == 0
  bltz      a8,skip       // skip if right_hand_x < 0
  blt       a8,a7,right_ok_m  // don't shorten line if right_hand_x < m_screen_width
  mov       a8,a7         // a8 (right_hand_x) <-- m_screen_width
right_ok_m:

  sub       a3,a8,a5      // a3 <-- final line width
  beqz      a3,skip_m     // skip if nothing to draw
  bltz      a3,skip_m     // skip if nothing to draw
  l32i      a14,a2,32     // a14 <-- m_bytes_per_line
  mul16u    a14,a14,a12   // a14 <-- m_bytes_per_line * (m_scrolled_y - m_y)
  addi      a14,a14,44    // a14 <-- offset within m_pixels array
  add       a14,a14,a2    // a14 <-- pointer to bitmap data line
  add       a14,a14,a13   // a14 <-- pointer to next bitmap pixel
  movi      a6,3          // a6 <-- mask for checking byte alignment
  movi      a8,2          // value for adjusting index
  movi      a10,1         // used to decrement width by 1
  movi      a11,4         // used to decrement width by 4
  movi      a12,0x3F3F3F3F // mask for clearing alpha bits

align_left_edge_m:
  beqz      a3,skip       // skip if no pixels left to draw
  and       a7,a14,a6     // a7 <-- (ptr to bitmap pixel) & 3
  bnez      a7,unaligned_m  // go if src pixel address is not aligned
  and       a7,a5,a6      // a7 <-- x & 3
  beqz      a7,left_aligned_m // don't align if already aligned
unaligned_m:
  xor       a9,a5,a8      // a9 <-- x ^ 2
  add       a9,a9,a4      // a9 <-- m_line8 + x
  l8ui      a2,a14,0      // a2 <-- pixel from bitmap
  bbsi      a2,6,show_pixel_m // go if pixel must be shown
  bbci      a2,7,dont_show_m // go if pixel must not be shown
show_pixel_m:
  and       a2,a2,a12     // clear alpha bits
  s8i       a2,a9,0       // m_line8[index] <-- m_color
dont_show_m:
  add       a14,a14,a10   // a14 <-- pointer to next bitmap pixel
  sub       a3,a3,a10     // a3 <-- remaining line width
  add       a5,a5,a10     // a5 <-- next x position
  bgez      a5,align_left_edge // go check alignment
left_aligned_m:

//  blt       a3,a11,middle_done // go if no full words to set
//do_middle:
//  add       a9,a5,a4      // a9 <-- m_line8 + x
//  l32i      a2,a14,0      // a2 <-- 4 pixels from bitmap
//  slli      a7,a2,16      // a7 <-- lower-half-word in upper-half-word position
//  srli      a2,a2,16      // a2 <-- upper-half-word in lower-half-word position
//  or        a2,a2,a7      // a2 <-- half-words swapped
//  and       a2,a2,a12     // clear alpha bits
//  s32i      a2,a9,0       // m_line32[index] <-- m_color (4 pixels)
//  add       a14,a14,a11   // a14 <-- pointer to next bitmap pixel
//  sub       a3,a3,a11     // a3 <-- remaining line width
//  add       a5,a5,a11     // a5 <-- next x position
//  bge       a3,a11,do_middle // go if more full words to set
//middle_done:

right_edge_m:
  beqz      a3,skip_m     // skip if no pixels left to draw
  xor       a9,a5,a8      // a9 <-- x ^ 2
  add       a9,a9,a4      // a9 <-- m_line8 + x
  l8ui      a2,a14,0      // a2 <-- pixel from bitmap
  bbsi      a2,6,show_pixel2_m // go if pixel must be shown
  bbci      a2,7,dont_show2_m // go if pixel must not be shown
show_pixel2_m:
  and       a2,a2,a12     // clear alpha bits
  s8i       a2,a9,0       // m_line8[index] <-- m_color
dont_show2_m:
  add       a14,a14,a10   // a14 <-- points to next bitmap pixel
  sub       a3,a3,a10     // a3 <-- remaining line width
  add       a5,a5,a10     // a5 <-- next x position
  bgez      a5,right_edge_m // go finish drawing

skip_m:
  retw

//----------------------------------------------------------------

