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
