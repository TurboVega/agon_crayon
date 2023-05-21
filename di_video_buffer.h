// di_video_buffer.h - Function declarations for painting video scan lines
//
// A a video buffer is a set of 1-pixel-high video scan lines that are equal
// in length (number of pixels) to the width of the video screen.
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
