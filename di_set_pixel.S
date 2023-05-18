// The function defined in this source may be specified
// in a C source that calls it, using this declaration:
//
// extern "C" {
// void IRAM_ATTR DiSetPixel_paint(const DiPaintParams *params);
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

  .section .iram1.text
  .align
  .global   DiSetPixel_paint
  .type     DiSetPixel_paint,@function

DiSetPixel_paint:
  entry     sp,16
  retw.n
