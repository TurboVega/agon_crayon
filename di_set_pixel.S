// The function defined in this source may be specified
// in a C source that calls it, using this declaration:
//
// extern "C" {
// IRAM_ATTR void DiSetPixel_paint(void* this_ptr, const DiPaintParams *params);
// }
//
// Upon entry to the function, registers are set as follows:
//
// a0: return address
// a1: stack pointer
// a2: DiSetPixel* pointer (i.e., 'this')
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

  .section .iram1.text
  .align
  .global   DiSetPixel_paint
  .type     DiSetPixel_paint,@function

DiSetPixel_paint:
  entry     sp,16
  l32i      a4,a3,12      // a4 <= m_scrolled_index
  l32i      a5,a2,8       // a5 <= m_y
  bne       a4,a5,skip    // skip if this is not line for pixel
  l32i      a4,a3,4       // a4 <= m_line8
  l32i      a5,a2,4       // a5 <= m_x
  l32i      a6,a3,16      // a6 <= m_horiz_scroll
  add       a5,a5,a6      // a5 <= m_x + m_horiz_scroll
  bltz      a5,skip       // skip if final x < 0
  l32i      a6,a3,24      // load screen width
  bge       a5,a6,skip    // skip if final x >= screen width
  movi      a6,2          // value for adjusting index
  xor       a5,a5,a6      // a5 <= final x ^ 2
  add       a4,a4,a5      // a4 <= m_line8 + m_x
  l8ui      a5,a2,12      // a5 <= m_color
  s8i       a5,a4,0       // m_line8[index] <= m_color
  memw
skip:
  retw.n
