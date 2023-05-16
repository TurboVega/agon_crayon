#include "di_drawing_instruction.h"

void IRAM_ATTR DiDrawingInstruction::paint(uint32_t* line, int32_t scroll) {
}

DiDrawingInstrAtX::DiDrawingInstrAtX() {
  m_x = 0;
}

DiDrawingInstrAtX::DiDrawingInstrAtX(int32_t x) {
  m_x = x;
}

DiDrawingInstrAtXY::DiDrawingInstrAtXY() {
  m_y = 0;
}

DiDrawingInstrAtXY::DiDrawingInstrAtXY(int32_t x, int32_t y) :
  DiDrawingInstrAtX(x) {
  m_y = y;
}
