// di_transparent_bitmap_asm.S - Low-level assembler function for drawing transparent bitmaps 
//
// A transparent bitmap is a rectangle that is a combination of fully transparent pixels,
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
// The function defined in this source may be specified
// in a C source that calls it, using this declaration:
//
// extern "C" {
// IRAM_ATTR void DiTransparentBitmap_paint(void* this_ptr, const DiPaintParams *params);
// }
//
// Upon entry to the function, registers are set as follows:
//
// a0: return address
// a1: stack pointer
// a2: DiTransparentBitmap* pointer (i.e., 'this')
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
//     [a2+44] m_visible_start
//     [a2+48] m_scroll_mode
//     [a2+52] m_pixels
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
  .global   DiTransparentBitmap_paint
  .type     DiTransparentBitmap_paint,@function

// Assume that the logical pixels in one line of the original source bitmap are
// arranged like this:
//
// [Start pixels] [Middle pixels] [End pixels]
//   0  1  2  3      4  5  6  7     8  9 10 11
//   a  b  c  d      m  n  o  p     w  x  y  z
//
// This means that the physical pixels (data bytes) in the stored bitmap are
// arranged like this, because of how the I2S hardware expects to see them
// for DMA purposes:
//
// [Start pixels] [Middle pixels] [End pixels]
//   0  1  2  3      4  5  6  7     8  9 10 11
//   c  d  a  b      o  p  m  n     y  z  w  x
//
// Contained in the m_pixels array are 4 copies of the bitmap pixel data.
// Each copy has its pixels horizontally offset by a different number of
// pixels (i.e., by a different number of bytes, since each byte contains
// a single pixel). The 4 positions of pixel data in the array are:
//
// Position A: pixels are at offset 0, and not not shifted to the right or to the left.
// Position B: pixels are at offset 1, being shifted to the right by 1 pixel, which is
//             similar to being shifted to the left by 3 pixels.
// Position C: pixels are at offset 2, being shifted to the right by 2 pixels, which is
//             similar to being shifted to the left by 2 pixels.
// Position D: pixels are at offset 3, being shifted to the right by 3 pixels, which is
//             similar to being shifted to the left by 1 pixel.
//
// When copying pixel data to the DMA scan line, both the source X position and
// the destination X position may be located at any of the 4 pixel offsets, from
// a word (4-byte) aligned boundary. The following table shows which copy of the
// pixel data to use when copying source pixels to destination pixels. The first
// arrangement in each pair is the logical arrangement, and the second arrangement
// in each pair is the physical arrangement.
//
// Dst offset-->   --- Dst offset 0 ----   ------ Dst offset 1 ------  ------ Dst offset 2 ------  ------ Dst offset 3 ------
//
// Src offset 0:   A:abcd.mnop.mnop.wxyz   B:-abc.dmno.pmno.pwxy.z---  C:--ab.cdmn.opmn.opwx.yz--  D:---a.bcdm.nopm.nopw.xzy-
//                 A:cdab.opmn.opmn.yzwx   B:bc-a.nodm.nopm.xypw.--z-  C:ab--.mncd.mnop.wxop.--yz  D:-a--.dmbc.pmno.pwno.y-xz
//
// Src offset 1:   D:bcdm.nopm.nopw.xyz-   A:-bcd.mnop.mnop.wxyz.----  B:--bc.dmno.pmno.pwxy.z---  C:---b.cdmn.opmn.opwx.yz--
//                 D:dmbc.pmno.pwno.z-xy   A:cd-b.opmn.opmn.yzwx.----  B:bc--.nodm.nopm.xypw.--z-  C:-b--.mncd.mnop.wxop.--yz
//
// Src offset 2:   C:cdmn.opmn.opwx.yz--   D:-cdm.nopm.nopw.xyz-.----  A:--cd.mnop.mnop.wxyz.----  B:---c.dmno.pmno.pwxy.z---
//                 C:mncd.mnop.wxop.--yz   D:dm-c.pmno.pwno.z-xy.----  A:cd--.opmn.opmn.yzwx.----  B:-c--.nodm.nopm.xypw.z---
//
// Src offset 3:   B:dmno.pmno.pwxy.z---   C:-dmn.opmn.opwx.yz--.----  D:--dm.nopm.nopw.xyz-.----  A:---d.mnop.mnop.wxyz.----
//                 B:nodm.nopm.xypw.--z-   C:mn-d.mnop.wxop.--yz.----  D:dm--.pmno.pwno.z-xy.----  A:-d--.opmn.opmn.yzwx.----
//

  .macro    LOOKUP_MIXED_COLOR c1 c0

  mov       a9,a2           // a9 <-- color lookup table
  bbci      a10,\c1,la\@    // go if FG upper bit is 0
  srli      a9,a9,16        // a9 <-- use upper half of table
  la\@:
  bbci      a10,\c0,lb\@    // go if FG lower bit is 0
  srli      a9,a9,8         // a9 <-- use upper quarter (half of chosen half)
  lb\@:
  bbci      a11,\c1,lc\@    // go if BG upper bit is 0
  srli      a9,a9,4         // a9 <-- use upper eighth (half of chosen quarter)
  lc\@:
  bbci      a11,\c0,ld\@    // go if BG lower bit is 0
  srli      a9,a9,2         // a9 <-- use upper sixteenth (half of chosen eighth)
  ld\@:
  src       a3,a9,a3        // a3 <-- slide 2 result color bits into holding register

  .endm

  .macro    MIX_COLOR_BYTE  alpha1 alpha0 b1 b0 g1 g0 r1 r0

  bbci      a10,\alpha1,le\@  // go if Alpha upper bit is 0
  bbci      a10,\alpha0,lf\@  // go if Alpha lower bit is 0

  // Alpha is 11 (3), meaning 100% opaque
  movi a9,0x3F
  ssai 8
  src  a3,a9,a3
  j         mix_done\@

  // Alpha is 10 (2), meaning 66% opaque
lf\@:
  movi      a2,0xEAA55440     // a2 <-- color lookup table (11101010101001010101010001000000)
  ssai      2                 // sar <-- setup to shift by 2 bits
  LOOKUP_MIXED_COLOR \r1,\r0  // mix the red color component
  LOOKUP_MIXED_COLOR \g1,\g0  // mix the green color component
  LOOKUP_MIXED_COLOR \b1,\b0  // mix the blue color component
  srli      a3,a3,2           // include 2 bits for VS & HS
  j         mix_done\@

le\@:
  bbci      a10,\alpha0,lg\@  // go if Alpha lower bit is 0

  // Alpha is 01 (1), meaning 50% opaque
  movi      a2,0xE9A59450     // a2 <-- color lookup table (11101001101001011001010001010000)
  ssai      2                 // sar <-- setup to shift by 2 bits
  LOOKUP_MIXED_COLOR \r1,\r0  // mix the red color component
  LOOKUP_MIXED_COLOR \g1,\g0  // mix the green color component
  LOOKUP_MIXED_COLOR \b1,\b0  // mix the blue color component
  srli      a3,a3,2           // include 2 bits for VS & HS
  j         mix_done\@

  // Alpha is 00 (0), meaning 33% or 0% opaque
lg\@:
  movi      a2,0xE5A49490     // a2 <-- color lookup table (11100101101001001001010010010000)
  ssai      2                 // sar <-- setup to shift by 2 bits
  LOOKUP_MIXED_COLOR \r1,\r0  // mix the red color component
  LOOKUP_MIXED_COLOR \g1,\g0  // mix the green color component
  LOOKUP_MIXED_COLOR \b1,\b0  // mix the blue color component
  srli      a3,a3,2           // include 2 bits for VS & HS

mix_done\@:

  .endm

  .align    4
DiTransparentBitmap_paint:
  entry     sp,16
  l32i      a4,a3,12      // a4 <-- m_scrolled_y
  l32i      a5,a2,8       // a5 <-- m_y
  blt       a4,a5,skip2   // skip if scan line is above bitmap area
  l32i      a6,a2,24      // a6 <-- m_y_extent
  bge       a4,a6,skip2   // skip if scan line is below bitmap area
  sub       a12,a4,a5     // a12 <-- m_scrolled_y - m_y
  l32i      a4,a3,4       // a4 <-- m_line8

  l32i      a5,a2,4       // a5 <-- m_x
  l32i      a6,a3,16      // a6 <-- m_horiz_scroll
  add       a5,a5,a6      // a5 (left_hand_x) <-- m_x + m_horiz_scroll
  mov       a13,a5        // a13 (left_hand_x) <-- m_x + m_horiz_scroll
  l32i      a7,a3,24      // a7 <-- m_screen_width
  mov       a3,a13        // a3 <-- left_hand_x
  bge       a5,a7,skip2   // skip if left_hand_x >= m_screen_width
  bgez      a5,left_ok    // don't shorten line if left_hand_x >= 0
  xor       a5,a5,a5      // a5 (left_hand_x) <-- 0
left_ok:
  sub       a13,a5,a13    // a13 <-- offset to first visible pixel
  l32i      a8,a2,16      // a8 <-- m_x_extent  
  add       a8,a8,a6      // a8 (right_hand_x) <-- m_x_extent + m_horiz_scroll
  beqz      a8,skip2      // skip if right_hand_x == 0
  bltz      a8,skip2      // skip if right_hand_x < 0
  blt       a8,a7,right_ok  // don't shorten line if right_hand_x < m_screen_width
  mov       a8,a7         // a8 (right_hand_x) <-- m_screen_width
right_ok:

  l32i      a7,a2,44      // a7 <-- m_visible_start
//  bgez      a3,dont_move  // don't move start if left_hand_x >= 0
//  bbsi      a3,0,move_start // go move start if left_hand_x is not a multiple of 4
//  bbci      a3,1,dont_move // do not move start if left_hand_x is a multiple of 4
//move_start:
//  //addi      a7,a7,4       // move start pixels address by 1 word
//dont_move:

  sub       a3,a8,a5      // a3 <-- final line width
  beqz      a3,skip2      // skip if nothing to draw
  bltz      a3,skip2      // skip if nothing to draw

  l32i      a14,a2,32     // a14 <-- m_bytes_per_line
  mul16u    a14,a14,a12   // a14 <-- m_bytes_per_line * (m_scrolled_y - m_y)
  add       a14,a14,a7    // a14 <-- pointer to bitmap data line
  l32i      a11,a2,40     // a11 <-- m_bytes_per_position
  movi      a7,1          // a7 <-- value of 1 for adjusting
  movi      a8,2          // a8 <-- value of 2 for adjusting
  movi      a9,3          // a9 <-- value of 3 for adjusting

  movi      a15,0xFFFFFFFC // a15 <-- mask to remove lower 2 bits
  and       a6,a15,a13    // a6 <-- offset to word-aligned src pixels
  add       a14,a14,a6    // a14 <-- points to word-aligned src pixels
  and       a6,a15,a5     // a6 <-- offset to word-aligned dst pixels
  add       a4,a4,a6      // a4 <-- points to word-aligned dst pixels

  mov       a15,a0        // a15 <-- save return address

//.......................................
// Determine the alignment of the source and destination pixels.
// a13 <-- offset to first visible src pixel

  bbsi      a13,1,src_offset_2_or_3     // go if first visible pixel is at offset 2 or 3
  bbsi      a13,0,src_offset_1          // go if first visible pixel is at offset 1
  bbci      a13,0,src_offset_0          // go, because first visible pixel is at offset 0
src_offset_2_or_3:
  bbsi      a13,0,src_offset_3          // go if first visible pixel is at offset 3
  bbci      a13,0,src_offset_2          // go, because first visible pixel is at offset 2

skip2:
  retw

// At this point:
// a3 <-- final line width
// a4 <-- m_line8 (or m_line32) + offset to word-aligned src pixels
// a5 <-- final start x
// a13 <-- offset to first visible src pixel
// a14 <-- pointer to bitmap data line

//.......................................

src_offset_0:
  bbsi     a5,1,so_0_dst_offset_2_or_3  // go if first destination pixel is at offset 2 or 3
  bbsi     a5,0,so_0_dst_offset_1       // go if first destination pixel is at offset 1

// Copy source pixels from offset 0 to destination pixels at offset 0.
so_0_dst_offset_0:
  // Logical  A:abcd.mnop.mnop.wxyz
  // Physical A:cdab.opmn.opmn.yzwx
  // a14 <-- pointer to source pixels (position A)
  call0    do_middle_pixels
  mov      a0,a15
  retw

// Copy source pixels from offset 0 to destination pixels at offset 1.
so_0_dst_offset_1:
  // Logical  B:-abc.dmno.pmno.pwxy.z---
  // Physical B:bc-a.nodm.nopm.xypw.--z-
  add      a14,a14,a11                  // a14 <-- pointer to source pixels (position B)
  //DO_START_PIXELS 0x00FF0000
  addi     a5,a5,3
  sub      a3,a3,a9
  call0    do_middle_pixels
  //DO_START_PIXELS 0xFF00FFFF
  mov      a0,a15
  retw

so_0_dst_offset_2_or_3:
  bbsi     a5,0,so_0_dst_offset_3       // go if first destination pixel is at offset 3

// Copy source pixels from offset 0 to destination pixels at offset 2.
so_0_dst_offset_2:
  // Logical  C:--ab.cdmn.opmn.opwx.yz--
  // Physical C:ab--.mncd.mnop.wxop.--yz
  add      a14,a14,a11                  // a14 <-- pointer to source pixels (position B)
  add      a14,a14,a11                  // (position C)
  //DO_START_PIXELS 0xFFFF0000
  addi     a5,a5,2
  sub      a3,a3,a8
  call0    do_middle_pixels
  //DO_START_PIXELS 0x0000FFFF
  mov      a0,a15
  retw

// Copy source pixels from offset 0 to destination pixels at offset 3.
so_0_dst_offset_3:
  // Logical  D:---a.bcdm.nopm.nopw.xzy-
  // Physical D:-a--.dmbc.pmno.pwno.y-xz
  add      a14,a14,a11                  // a14 <-- pointer to source pixels (position B)
  add      a14,a14,a11                  // (position C)
  add      a14,a14,a11                  // (position D)
  //DO_START_PIXELS 0xFFFF00FF
  addi     a5,a5,1
  sub      a3,a3,a7
  call0    do_middle_pixels
  //DO_START_PIXELS 0x0000FF00
  mov      a0,a15
  retw

//.......................................

src_offset_1:
  bbsi     a5,1,so_1_dst_offset_2_or_3  // go if first destination pixel is at offset 2 or 3
  bbsi     a5,0,so_1_dst_offset_1       // go if first destination pixel is at offset 1

// Copy source pixels from offset 1 to destination pixels at offset 0.
so_1_dst_offset_0:
  // Logical  D:bcdm.nopm.nopw.xyz-
  // Physical D:dmbc.pmno.pwno.z-xy
  add      a14,a14,a11                  // a14 <-- pointer to source pixels (position B)
  add      a14,a14,a11                  // (position C)
  add      a14,a14,a11                  // (position D)
  call0    do_middle_pixels
  //DO_START_PIXELS 0x0000FF00
  mov      a0,a15
  retw

// Copy source pixels from offset 1 to destination pixels at offset 1.
so_1_dst_offset_1:
  // Logical  A:-bcd.mnop.mnop.wxyz.----
  // Physical A:cd-b.opmn.opmn.yzwx.----
  // a14 <-- pointer to source pixels (position A)
  //DO_START_PIXELS 0x00FF0000
  addi     a5,a5,3
  sub      a3,a3,a9
  call0    do_middle_pixels
  mov      a0,a15
  retw

so_1_dst_offset_2_or_3:
  bbsi     a5,0,so_1_dst_offset_3     // go if first destination pixel is at offset 3

// Copy source pixels from offset 1 to destination pixels at offset 2.
so_1_dst_offset_2:
  // Logical  B:--bc.dmno.pmno.pwxy.z---
  // Physical B:bc--.nodm.nopm.xypw.--z-
  add      a14,a14,a11                  // a14 <-- pointer to source pixels (position B)
  //DO_START_PIXELS 0xFFFF0000
  addi     a5,a5,2
  sub      a3,a3,a8
  call0    do_middle_pixels
  //DO_START_PIXELS 0xFF00FFFF
  mov      a0,a15
  retw

// Copy source pixels from offset 1 to destination pixels at offset 3.
so_1_dst_offset_3:
  // Logical  C:---b.cdmn.opmn.opwx.yz--
  // Physical C:-b--.mncd.mnop.wxop.--yz
  add      a14,a14,a11                  // a14 <-- pointer to source pixels (position B)
  add      a14,a14,a11                  // (position C)
  //DO_START_PIXELS 0xFFFF00FF
  addi     a5,a5,1
  sub      a3,a3,a7
  call0    do_middle_pixels
  //DO_START_PIXELS 0x0000FFFF
  mov      a0,a15
  retw

//.......................................

src_offset_2:
  bbsi     a5,1,so_2_dst_offset_2_or_3  // go if first destination pixel is at offset 2 or 3
  bbsi     a5,0,so_2_dst_offset_1       // go if first destination pixel is at offset 1

// Copy source pixels from offset 2 to destination pixels at offset 0.
so_2_dst_offset_0:
  // Logical  C:cdmn.opmn.opwx.yz--
  // Physical C:mncd.mnop.wxop.--yz
  add      a14,a14,a11                  // a14 <-- pointer to source pixels (position B)
  add      a14,a14,a11                  // (position C)
  call0    do_middle_pixels
  //DO_START_PIXELS 0x0000FFFF
  mov      a0,a15
  retw

// Copy source pixels from offset 2 to destination pixels at offset 1.
so_2_dst_offset_1:
  // Logical  D:-cdm.nopm.nopw.xyz-.----
  // Physical D:dm-c.pmno.pwno.z-xy.----
  add      a14,a14,a11                  // a14 <-- pointer to source pixels (position B)
  add      a14,a14,a11                  // (position C)
  add      a14,a14,a11                  // (position D)
  //DO_START_PIXELS 0x00FF0000
  addi     a5,a5,3
  sub      a3,a3,a9
  call0    do_middle_pixels
  //DO_START_PIXELS 0x0000FF00
  mov      a0,a15
  retw

so_2_dst_offset_2_or_3:
  bbsi     a5,0,so_2_dst_offset_3       // go if first destination pixel is at offset 3

// Copy source pixels from offset 2 to destination pixels at offset 2.
so_2_dst_offset_2:
  // Logical  A:--cd.mnop.mnop.wxyz.----
  // Physical A:cd--.opmn.opmn.yzwx.----
  // a14 <-- pointer to source pixels (position A)
  //DO_START_PIXELS 0xFFFF0000
  addi     a5,a5,2
  sub      a3,a3,a8
  call0    do_middle_pixels
  mov      a0,a15
  retw

// Copy source pixels from offset 2 to destination pixels at offset 3.
so_2_dst_offset_3:
  // Logical  B:---c.dmno.pmno.pwxy.z---
  // Physical B:-c--.nodm.nopm.xypw.z---
  add      a14,a14,a11                  // a14 <-- pointer to source pixels (position B)
  //DO_START_PIXELS 0xFFFF00FF
  addi     a5,a5,1
  sub      a3,a3,a7
  call0    do_middle_pixels
  //DO_START_PIXELS 0xFFFFFF00
  mov      a0,a15
  retw

//.......................................

src_offset_3:
  bbsi     a5,1,so_3_dst_offset_2_or_3  // go if first destination pixel is at offset 2 or 3
  bbsi     a5,0,so_3_dst_offset_1       // go if first destination pixel is at offset 1

// Copy source pixels from offset 3 to destination pixels at offset 0.
so_3_dst_offset_0:
  // Logical  B:dmno.pmno.pwxy.z---
  // Physical B:nodm.nopm.xypw.--z-
  add      a14,a14,a11                  // a14 <-- pointer to source pixels (position B)
  //sub      a5,a5,a7
  call0    do_middle_pixels
  //DO_START_PIXELS 0xFF00FFFF
  mov      a0,a15
  retw

// Copy source pixels from offset 3 to destination pixels at offset 1.
so_3_dst_offset_1:
  // Logical  C:-dmn.opmn.opwx.yz--.----
  // Physical C:mn-d.mnop.wxop.--yz.----
  add      a14,a14,a11                  // a14 <-- pointer to source pixels (position B)
  add      a14,a14,a11                  // (position C)
  //DO_START_PIXELS 0x00FF0000
  addi     a5,a5,3
  sub      a3,a3,a9
  call0    do_middle_pixels
  //DO_START_PIXELS 0x0000FFFF
  mov      a0,a15
  retw

so_3_dst_offset_2_or_3:
  bbsi     a5,0,so_3_dst_offset_3       // go if first destination pixel is at offset 3

// Copy source pixels from offset 3 to destination pixels at offset 2.
so_3_dst_offset_2:
  // Logical  D:--dm.nopm.nopw.xyz-.----
  // Physical D:dm--.pmno.pwno.z-xy.----
  add      a14,a14,a11                  // a14 <-- pointer to source pixels (position B)
  add      a14,a14,a11                  // (position C)
  add      a14,a14,a11                  // (position D)
  //DO_START_PIXELS 0xFFFF0000
  addi     a5,a5,2
  sub      a3,a3,a8
  call0    do_middle_pixels
  //DO_END_PIXELS 0x0000FF00
  mov      a0,a15
  retw

// Copy source pixels from offset 3 to destination pixels at offset 3.
so_3_dst_offset_3:
  // Logical  A:---d.mnop.mnop.wxyz.----
  // Physical A:-d--.opmn.opmn.yzwx.----
  // a14 <-- pointer to source pixels (position A)
  //DO_START_PIXELS 0xFFFF00FF
  addi     a5,a5,1
  sub      a3,a3,a7
  call0    do_middle_pixels
  mov      a0,a15
  retw

//.......................................
// a3 <-- number of bytes left to copy, then scratch
// a4 <-- m_line8 (or m_line32)
// a14 <-- pointer to bitmap data line
// a2, a3, a5, a6, a7, a8, a9, a10, a11, a12, a13 <-- scratch

do_middle_pixels:
  movi      a7,0xC0C0C0C0 // a7 <-- mask to isolate transparency bits
  movi      a8,0x80808080 // a8 <-- mask to isolate upper transparency bit
  movi      a12,0x40404040 // a12 <-- mask to isolate lower transparency bit

  srli      a3,a3,2       // a3 <-- number of full words left to copy
  loopnez   a3,end_loop   // loop to copy pixel data

movi a3,0x01040506
j write_pix
  l32i      a10,a14,0     // a10 <-- read 4 src pixels
  and       a11,a10,a7      // a11 <-- src transparency bits only (no color)
  beq       a11,a7,pct_100  // go if using all 100% opaquness
  beqz      a11,pct_33      // go if using all 33% or 0% opaquness
  beq       a11,a12,pct_50  // go if using all 50% opaquness

// The transparency bits are dissimilar among the source pixels,
// or all pixels are either 33% or 66% opaque.
// We must handle the 4 pixels separately.
pct_oddball:
  l32i      a11,a4,0      // a11 <-- read 4 dst pixels
  movi      a3,0          // a3 <-- output begins as 4 black pixels
  ssai      8             // sar <-- will shift in 8 bits via 'src', below
  movi      a3,0x20          // a3 <-- output begins as 4 black pixels
  j         write_pix     // go write the resulting dst pixels

  //MIX_COLOR_BYTE 7,6,5,4,3,2,1,0          // mix byte # of the pixel word
  //MIX_COLOR_BYTE 15,14,13,12,11,10,9,8    // mix byte #1 of the pixel word
  //MIX_COLOR_BYTE 23,22,21,20,19,18,17,16  // mix byte #2 of the pixel word
  //MIX_COLOR_BYTE 31,30,29,28,27,26,25,24  // mix byte #3 of the pixel word

  // xxxx xxxx xxxx xxxx xxxx xxxx TTBB GGRR
  // xxxx xxxx xxxx xxxx xxTT BBGG RR00 0000
  movi      a5,0x0000003F // a5 <-- mask to separate dst pixel bits
  and       a6,a11,a5     // a6 <-- keep only dst pixel bits
  l32i      a9,a14,4      // a9 <-- points to mixed color table
  add       a9,a9,a6      // a9 <-- compute address of table element
  l8ui      a9,a9,0       // a9 <-- load mixed color value
  src       a3,a9,a3      // a3 <-- shifts in 1st new pixel (8 bits)

  // xxxx xxxx xxxx xxxx TTBB GGRR xxxx xxxx
  // xxxx xxxx xxxx xxxx xxTT BBGG RR00 0000
  srli      a11,a11,8     // a11 <-- move next dst pixels to lower bits
  and       a6,a11,a5     // a6 <-- keep only dst pixel bits
  l32i      a9,a14,8      // a9 <-- points to mixed color table
  add       a9,a9,a6      // a9 <-- compute address of table element
  l8ui      a9,a9,0       // a9 <-- load mixed color value
  src       a3,a9,a3      // a3 <-- shifts in 2nd new pixel (8 bits)

  // xxxx xxxx TTBB GGRR xxxx xxxx xxxx xxxx
  // xxxx xxxx xxxx xxxx xxTT BBGG RR00 0000
  srli      a11,a11,8     // a11 <-- move next dst pixels to lower bits
  and       a6,a11,a5     // a6 <-- keep only dst pixel bits
  l32i      a9,a14,12     // a9 <-- points to mixed color table
  add       a9,a9,a6      // a9 <-- compute address of table element
  l8ui      a9,a9,0       // a9 <-- load mixed color value
  src       a3,a9,a3      // a3 <-- shifts in 3rd new pixel (8 bits)

  // TTBB GGRR xxxx xxxx xxxx xxxx xxxx xxxx
  // xxxx xxxx xxxx xxxx xxTT BBGG RR00 0000
  srli      a11,a11,8     // a11 <-- move next dst pixels to lower bits
  and       a6,a11,a5     // a6 <-- keep only dst pixel bits
  l32i      a9,a14,16     // a9 <-- points to mixed color table
  add       a9,a9,a6      // a9 <-- compute address of table element
  l8ui      a9,a9,0       // a9 <-- load mixed color value
  src       a3,a9,a3      // a3 <-- shifts in 4th new pixel (8 bits)

  j         write_pix     // go write the resulting dst pixels

// All 4 src pixels are 100% opaque.
pct_100:
  xor       a3,a10,a7     // a3 <-- get rid of the transparency bits, leaving color
  j         write_pix     // go write the resulting dst pixels

// All 4 src pixels are 50% opaque.
pct_50:
 movi a3,0x14
 j         write_pix     // go write the resulting dst pixels
  l32i      a11,a4,0      // a11 <-- read 4 dst pixels

  movi      a5,0x33333333 // a5 <-- mask to isolate blue & red, removing green
  and       a9,a10,a5     // a9 <-- src blue & red (no green)
  and       a13,a11,a5    // a13 <-- dst blue & red (no green)
  add       a13,a13,a9    // a13 <-- sum of blue & sum of red
  srli      a13,a13,1     // a13 <-- averages of mixed blue & mixed red, in 3 bits each
  and       a13,a13,a5    // a13 <-- averages in 2 bits each

  movi      a6,0x0C0C0C0C // a6 <-- mask to isolate green, removing red & blue
  and       a10,a10,a6    // a10 <-- src green (no red or blue)
  and       a11,a11,a6    // a11 <-- dst green (no red or blue)
  add       a11,a11,a10   // a11 <-- sum of green
  srli      a11,a11,1     // a11 <-- averages of mixed green, in 3 bits each
  and       a11,a11,a6    // a11 <-- averages in 2 bits each

  or        a3,a13,a11    // a3 <-- 4 mixed pixels
  j         write_pix     // go write the resulting dst pixels

// All 4 src pixels are 0% or 33% opaque.
pct_33:
  l32i      a11,a4,0      // a11 <-- read 4 dst pixels
  movi a3,0x22 // TEMP
  j         write_pix     // go write the resulting dst pixels

write_pix:
  s32i      a3,a4,0       // write 4 dst pixels
pct_0:
  addi      a4,a4,4       // a4 <-- points to next 4 dst pixels
  addi      a14,a14,20    // a14 <-- points to next 4 src pixels
end_loop:
  ret
