#pragma once
#include "di_drawing_instruction.h"

class DiVideoScanLine {
  protected:

  uint32_t m_act[ACT_PIXELS/4];
  uint32_t m_hfp[HFP_PIXELS/4];
  uint32_t m_hs[HS_PIXELS/4];
  uint32_t m_hbp[HBP_PIXELS/4];

  public:

  inline uint32_t get_buffer_size() {
    return sizeof(m_act) + sizeof(m_hfp) + sizeof(m_hs) + sizeof(m_hbp);
  }

  inline uint32_t volatile * get_buffer_ptr() {
    return (uint32_t volatile *) m_act;
  }

  void init_to_black();

  void init_for_vsync();

  void IRAM_ATTR paint(DiPaintParams *params);
};

class DiVideoBuffer {
  protected:

  DiVideoScanLine m_line[NUM_LINES_PER_BUFFER];

  public:

  inline int32_t get_buffer_size() {
    return sizeof(m_line);
  }

  inline uint32_t volatile * get_buffer_ptr() {
    return (uint32_t volatile *) m_line;
  }

  void init_to_black();

  void init_for_vsync();

  void IRAM_ATTR paint(DiPaintParams *params);
};
