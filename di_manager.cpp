// di_manager.cpp - Function definitions for managing drawing-instruction primitives
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

#include "di_manager.h"

#include <stddef.h>
#include "soc/i2s_struct.h"
#include "soc/i2s_reg.h"
#include "driver/periph_ctrl.h"
#include "soc/rtc.h"
#include "driver/gpio.h"
#include "soc/io_mux_reg.h"
#include "fabgl_pieces.h"

#include "di_diag_left_line.h"
#include "di_diag_right_line.h"
#include "di_general_line.h"
#include "di_horiz_line.h"
#include "di_set_pixel.h"
#include "di_vert_line.h"
#include "di_solid_rectangle.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/i2s_struct.h"
#include "soc/i2s_reg.h"
#include "ESP32Time.h"

DiManager::DiManager() {
}

DiManager::~DiManager() {
    clear();
}

void DiManager::initialize() {
  size_t new_size = (size_t)(sizeof(lldesc_t) * DMA_TOTAL_DESCR);
  void* p = heap_caps_malloc(new_size, MALLOC_CAP_32BIT|MALLOC_CAP_8BIT|MALLOC_CAP_DMA);
  m_dma_descriptor = (volatile lldesc_t *)p;

  new_size = (size_t)(sizeof(DiVideoBuffer) * NUM_ACTIVE_BUFFERS);
  p = heap_caps_malloc(new_size, MALLOC_CAP_32BIT|MALLOC_CAP_8BIT|MALLOC_CAP_DMA);
  m_video_buffer = (volatile DiVideoBuffer *)p;

  new_size = (size_t)(sizeof(DiVideoScanLine));
  p = heap_caps_malloc(new_size, MALLOC_CAP_32BIT|MALLOC_CAP_8BIT|MALLOC_CAP_DMA);
  m_front_porch = (volatile DiVideoScanLine *)p;

  new_size = (size_t)(sizeof(DiVideoBuffer));
  p = heap_caps_malloc(new_size, MALLOC_CAP_32BIT|MALLOC_CAP_8BIT|MALLOC_CAP_DMA);
  m_vertical_sync = (volatile DiVideoBuffer *)p;

  new_size = (size_t)(sizeof(DiVideoScanLine));
  p = heap_caps_malloc(new_size, MALLOC_CAP_32BIT|MALLOC_CAP_8BIT|MALLOC_CAP_DMA);
  m_back_porch = (volatile DiVideoScanLine *)p;

  // DMA buffer chain: ACT
  uint32_t descr_index = 0;
  for (uint32_t i = 0; i < NUM_ACTIVE_BUFFERS; i++) {
    m_video_buffer[i].init_to_black();
  }
  for (uint32_t i = 0; i < ACT_BUFFERS_WRITTEN; i++) {
    init_dma_descriptor(&m_video_buffer[i & (NUM_ACTIVE_BUFFERS - 1)], descr_index++);
  }

  // DMA buffer chain: VFP
  m_front_porch->init_to_black();
  for (uint i = 0; i < VFP_LINES; i++) {
    init_dma_descriptor(m_front_porch, descr_index++);
  }

  // DMA buffer chain: VS
  m_vertical_sync->init_for_vsync();
  for (uint i = 0; i < VS_LINES/NUM_LINES_PER_BUFFER; i++) {
    init_dma_descriptor(m_vertical_sync, descr_index++);
  }
  
  // DMA buffer chain: VBP
  m_back_porch->init_to_black();
  for (uint i = 0; i < VBP_LINES; i++) {
    init_dma_descriptor(m_back_porch, descr_index++);
  }

  // GPIO configuration for color bits
  setupGPIO(GPIO_RED_0,   VGA_RED_BIT,   GPIO_MODE_OUTPUT);
  setupGPIO(GPIO_RED_1,   VGA_RED_BIT + 1,   GPIO_MODE_OUTPUT);
  setupGPIO(GPIO_GREEN_0, VGA_GREEN_BIT, GPIO_MODE_OUTPUT);
  setupGPIO(GPIO_GREEN_1, VGA_GREEN_BIT + 1, GPIO_MODE_OUTPUT);
  setupGPIO(GPIO_BLUE_0,  VGA_BLUE_BIT,  GPIO_MODE_OUTPUT);
  setupGPIO(GPIO_BLUE_1,  VGA_BLUE_BIT + 1,  GPIO_MODE_OUTPUT);

  // GPIO configuration for VSync and HSync
  setupGPIO(GPIO_HSYNC, VGA_HSYNC_BIT, GPIO_MODE_OUTPUT);
  setupGPIO(GPIO_VSYNC, VGA_VSYNC_BIT, GPIO_MODE_OUTPUT);

  // Start the DMA

  // Power on device
  periph_module_enable(PERIPH_I2S1_MODULE);

  // Initialize I2S device
  I2S1.conf.tx_reset = 1;
  I2S1.conf.tx_reset = 0;

  // Reset DMA
  I2S1.lc_conf.out_rst = 1;
  I2S1.lc_conf.out_rst = 0;

  // Reset FIFO
  I2S1.conf.tx_fifo_reset = 1;
  I2S1.conf.tx_fifo_reset = 0;

  // LCD mode
  I2S1.conf2.val            = 0;
  I2S1.conf2.lcd_en         = 1;
  I2S1.conf2.lcd_tx_wrx2_en = 0; // NOT 1!
  I2S1.conf2.lcd_tx_sdx2_en = 0;

  I2S1.sample_rate_conf.val         = 0;
  I2S1.sample_rate_conf.tx_bits_mod = 8;

  setup_dma_clock(DMA_CLOCK_FREQ);

  I2S1.fifo_conf.val                  = 0;
  I2S1.fifo_conf.tx_fifo_mod_force_en = 1;
  I2S1.fifo_conf.tx_fifo_mod          = 1;
  I2S1.fifo_conf.tx_fifo_mod          = 1;
  I2S1.fifo_conf.tx_data_num          = 32;
  I2S1.fifo_conf.dscr_en              = 1;

  I2S1.conf1.val           = 0;
  I2S1.conf1.tx_stop_en    = 0;
  I2S1.conf1.tx_pcm_bypass = 1;

  I2S1.conf_chan.val         = 0;
  I2S1.conf_chan.tx_chan_mod = 1;

  I2S1.conf.tx_right_first = 0;

  I2S1.timing.val = 0;

  // Reset AHB interface of DMA
  I2S1.lc_conf.ahbm_rst      = 1;
  I2S1.lc_conf.ahbm_fifo_rst = 1;
  I2S1.lc_conf.ahbm_rst      = 0;
  I2S1.lc_conf.ahbm_fifo_rst = 0;

  // Start DMA
  I2S1.lc_conf.val = I2S_OUT_DATA_BURST_EN;// | I2S_OUTDSCR_BURST_EN;
  I2S1.out_link.addr = (uint32_t)m_dma_descriptor;
  I2S1.int_clr.val = 0xFFFFFFFF;
  I2S1.out_link.start = 1;
  I2S1.conf.tx_start  = 1;
}

void DiManager::clear() {
    // NOTE: add code to delete each prim only once!!
    for (int g = 0; g < NUM_VERTICAL_GROUPS; g++) {
        std::vector<DiPrimitive*> * vp = &m_groups[g];
        for (auto prim = vp->begin(); prim != vp->end(); ++prim) {
            delete *prim;
        }
        vp->clear();
    }

    heap_caps_free((void*)m_dma_descriptor);
    heap_caps_free((void*)m_video_buffer);
    heap_caps_free((void*)m_front_porch);
    heap_caps_free((void*)m_vertical_sync);
    heap_caps_free((void*)m_back_porch);
}

void DiManager::add_primitive(DiPrimitive* prim) {
    int32_t min_group, max_group;
    prim->get_vertical_group_range(&min_group, &max_group);
    for (int32_t g = min_group; g <= max_group; g++) {
        m_groups[g].push_back(prim);
    }
}

DiPrimitive* DiManager::create_point(int32_t x, int32_t y, uint8_t color) {
    DiPrimitive* prim = new DiSetPixel(x, y, color);
    add_primitive(prim);
    return prim;
}

DiPrimitive* DiManager::create_line(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint8_t color) {
    DiPrimitive* prim;
    if (x1 == x2) {
        if (y1 == y2) {
            prim = new DiSetPixel(x1, y1, color);
        } else if (y1 < y2) {
            prim = new DiVerticalLine(x1, y1, y2 - y1 + 1, color);
        } else {
            prim = new DiVerticalLine(x1, y2, y1 - y2 + 1, color);
        }
    } else if (x1 < x2) {
        if (y1 == y2) {
            prim = new DiHorizontalLine(x1, y1, x2 - x1 + 1, color);
        } else if (y1 < y2) {
            if (y2 - y1 == x2 - x1) {
                prim = new DiDiagonalRightLine(x1, y1, x2 - x1 + 1, color);
            } else {
                prim = new DiGeneralLine(x1, y1, x2, y2, color);
            }
        } else {
            if (y2 - y1 == x2 - x1) {
                prim = new DiDiagonalLeftLine(x2, y1, x2 - x1 + 1, color);
            } else {
                prim = new DiGeneralLine(x1, y1, x2, y2, color);
            }
        }
    } else {
        if (y1 == y2) {
            prim = new DiHorizontalLine(x2, y1, x1 - x2 + 1, color);
        } else if (y1 < y2) {
            if (y2 - y1 == x1 - x2) {
                prim = new DiDiagonalLeftLine(x1, y1, x1 - x2 + 1, color);
            } else {
                prim = new DiGeneralLine(x1, y1, x2, y2, color);
            }
        } else {
            if (y2 - y1 == x1 - x2) {
                prim = new DiDiagonalRightLine(x2, y1, x1 - x2 + 1, color);
            } else {
                prim = new DiGeneralLine(x1, y1, x2, y2, color);
            }
        }
    }

    add_primitive(prim);
    return prim;
}

DiPrimitive* DiManager::create_solid_rectangle(int32_t x, int32_t y, uint32_t width, uint32_t height, uint8_t color) {
    DiPrimitive* prim = new DiSolidRectangle(x, y, width, height, color);
    add_primitive(prim);
    return prim;
}

DiPrimitive* DiManager::create_triangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint8_t color) {
    DiPrimitive* prim = new DiGeneralLine(x1, y1, x2, y2, x3, y3, color);
    add_primitive(prim);
    return prim;
}

void IRAM_ATTR DiManager::run() {
    initialize();
    loop();
    clear();
}

void IRAM_ATTR DiManager::loop() {
  DiPaintParams paint_params;
  paint_params.m_horiz_scroll = 0;
  paint_params.m_vert_scroll = 0;
  paint_params.m_screen_width = ACT_PIXELS;
  paint_params.m_screen_height = ACT_LINES;

  uint32_t current_line_index = 0;//NUM_ACTIVE_BUFFERS * NUM_LINES_PER_BUFFER;
  uint32_t current_buffer_index = 0;
  bool end_of_frame = false;

  while (true) {
    uint32_t descr_addr = (uint32_t) I2S1.out_link_dscr;
    uint32_t descr_index = (descr_addr - (uint32_t)m_dma_descriptor) / sizeof(lldesc_t);
    if (descr_index <= ACT_BUFFERS_WRITTEN) {
      //uint32_t dma_line_index = descr_index * NUM_LINES_PER_BUFFER;
      uint32_t dma_buffer_index = descr_index & (NUM_ACTIVE_BUFFERS-1);

      // Draw enough lines to stay ahead of DMA.
      while (current_line_index < ACT_LINES && current_buffer_index != dma_buffer_index) {
        volatile DiVideoBuffer* vbuf = &m_video_buffer[current_buffer_index];
        paint_params.m_line_index = current_line_index;
        paint_params.m_scrolled_y = current_line_index + paint_params.m_vert_scroll;
        paint_params.m_line8 = (volatile uint8_t*) vbuf->get_buffer_ptr_0();
        paint_params.m_line32 = vbuf->get_buffer_ptr_0();
        draw_primitives(&paint_params);
        //memset((void*)paint_params.m_line8, 0x00, 800);
        paint_params.m_line8[current_line_index^2] = 0x01;
        paint_params.m_line8[(current_line_index&15)^2] = 0x30;

        paint_params.m_line_index = ++current_line_index;
        paint_params.m_scrolled_y = current_line_index + paint_params.m_vert_scroll;
        paint_params.m_line8 = (volatile uint8_t*) vbuf->get_buffer_ptr_1();
        paint_params.m_line32 = vbuf->get_buffer_ptr_1();
        draw_primitives(&paint_params);
        //memset((void*)paint_params.m_line8, 0x00, 800);
        paint_params.m_line8[current_line_index^2] = 0x01;
        paint_params.m_line8[(current_line_index&15)^2] = 0x030;

        ++current_line_index;
        if (++current_buffer_index >= NUM_ACTIVE_BUFFERS) {
          current_buffer_index = 0;
        }
      }
      end_of_frame = false;
    } else if (!end_of_frame) {
      // Handle modifying primitives before the next frame starts.
      on_vertical_blank();

      // Prepare the start of the next frame.
      for (current_line_index = 0, current_buffer_index = 0;
            current_buffer_index < NUM_ACTIVE_BUFFERS;
            current_line_index++, current_buffer_index++) {
        volatile DiVideoBuffer* vbuf = &m_video_buffer[current_buffer_index];
        paint_params.m_line_index = current_line_index;
        paint_params.m_scrolled_y = current_line_index + paint_params.m_vert_scroll;
        paint_params.m_line8 = (volatile uint8_t*) vbuf->get_buffer_ptr_0();
        paint_params.m_line32 = vbuf->get_buffer_ptr_0();
        draw_primitives(&paint_params);
        //memset((void*)paint_params.m_line8, 0x00, 800);
        paint_params.m_line8[201] = 0x03F;

        paint_params.m_line_index = ++current_line_index;
        paint_params.m_scrolled_y = current_line_index + paint_params.m_vert_scroll;
        paint_params.m_line8 = (volatile uint8_t*) vbuf->get_buffer_ptr_1();
        paint_params.m_line32 = vbuf->get_buffer_ptr_1();
        draw_primitives(&paint_params);
        //memset((void*)paint_params.m_line8, 0x00, 800);
        paint_params.m_line8[205] = 0x03F;
      }

      end_of_frame = true;
      current_line_index = 0;
      current_buffer_index = 0;
    }
  }
}

void IRAM_ATTR DiManager::draw_primitives(DiPaintParams* params) {
    int32_t g = params->m_line_index >> VERTICAL_GROUP_INDEX_SHIFT;
    std::vector<DiPrimitive*> * vp = &m_groups[g];
    for (auto prim = vp->begin(); prim != vp->end(); ++prim) {
        (*prim)->paint(params);
    }
}

void IRAM_ATTR DiManager::on_vertical_blank() {
    static bool created = false;
    if (!created) {
        created = true;
        create_samples();
    }
}

void DiManager::init_dma_descriptor(volatile DiVideoScanLine* vline, uint32_t descr_index) {
  volatile lldesc_t * dd = &m_dma_descriptor[descr_index];

  if (descr_index == 0) {
    m_dma_descriptor[DMA_TOTAL_DESCR - 1].qe.stqe_next = (lldesc_t*)dd;
  } else {
    m_dma_descriptor[descr_index - 1].qe.stqe_next = (lldesc_t*)dd;
  }

  dd->sosf = dd->offset = dd->eof = 0;
  dd->owner = 1;
  dd->size = vline->get_buffer_size();
  dd->length = vline->get_buffer_size();
  dd->buf = (uint8_t volatile *)vline->get_buffer_ptr();
}

void DiManager::init_dma_descriptor(volatile DiVideoBuffer* vbuf, uint32_t descr_index) {
  volatile lldesc_t * dd = &m_dma_descriptor[descr_index];

  if (descr_index == 0) {
    m_dma_descriptor[DMA_TOTAL_DESCR - 1].qe.stqe_next = (lldesc_t*)dd;
  } else {
    m_dma_descriptor[descr_index - 1].qe.stqe_next = (lldesc_t*)dd;
  }

  dd->sosf = dd->offset = dd->eof = 0;
  dd->owner = 1;
  dd->size = vbuf->get_buffer_size();
  dd->length = vbuf->get_buffer_size();
  dd->buf = (uint8_t volatile *)vbuf->get_buffer_ptr_0();
}

void DiManager::create_samples() {
  DiPrimitive* prim1 = create_solid_rectangle(0, 0, 800, 600, 0x00);
  //DiPrimitive* prim2a = create_line(2, 0, 797, 0, 0x33); // horiz
  //DiPrimitive* prim2b = create_line(0, 599, 799, 599, 0x13); // horiz
  //DiPrimitive* prim3a = create_line(0, 0, 0, 599, 0x35); // vert
  //DiPrimitive* prim3b = create_line(799, 0, 799, 599, 0x27); // vert

  DiPrimitive* prim3c = create_line(0, 0, 799, 599, 0x15); // diag

  DiPrimitive* prim10a = create_point(400, 0, 0x3F);
  DiPrimitive* prim10b = create_point(400, 599, 0x3F);

  //DiPrimitive* prim1 = create_solid_rectangle(0, 0, 800, 600, 0x01);

  DiPrimitive* prim2 = create_line(0, 19, 799, 19, 0x33); // horiz

  DiPrimitive* prim6 = create_line(50, 13, 75, 17, 0x1E);
  DiPrimitive* prim7 = create_line(750, 431, 786, 411, 0x1E);
  DiPrimitive* prim8 = create_solid_rectangle(150, 300, 227, 227, 0x20);

  DiPrimitive* prim10 = create_triangle(450, 330, 520, 402, 417, 375, 0x15);
  DiPrimitive* prim10d = create_line(450, 330, 520, 402, 0x30);
  DiPrimitive* prim10e = create_line(520, 402, 417, 375, 0x0C);
  DiPrimitive* prim10f = create_line(417, 375, 450, 330, 0x03);

  int32_t x = 0, y = 0;
  DiPrimitive* prim11a = create_line(x, y, x+15, y, 0x33);
  DiPrimitive* prim11b = create_line(x+15, y, x+15, y+15, 0x0F);
  DiPrimitive* prim11c = create_line(x+15, y+15, x, y+15, 0x3C);
  DiPrimitive* prim11d = create_line(x, y+15, x, y, 0x03);

  x = 799-15, y = 0;
  prim11a = create_line(x, y, x+15, y, 0x33);
  prim11b = create_line(x+15, y, x+15, y+15, 0x0F);
  prim11c = create_line(x+15, y+15, x, y+15, 0x3C);
  prim11d = create_line(x, y+15, x, y, 0x03);

  x = 799-15, y = 599-15;
  prim11a = create_line(x, y, x+15, y, 0x33);
  prim11b = create_line(x+15, y, x+15, y+15, 0x0F);
  prim11c = create_line(x+15, y+15, x, y+15, 0x3C);
  prim11d = create_line(x, y+15, x, y, 0x03);

  x = 0, y = 599-15;
  prim11a = create_line(x, y, x+15, y, 0x33);
  prim11b = create_line(x+15, y, x+15, y+15, 0x0F);
  prim11c = create_line(x+15, y+15, x, y+15, 0x3C);
  prim11d = create_line(x, y+15, x, y, 0x03);

  x = 600, y = 150;
  prim11a = create_line(x, y-75, x+75, y, 0x33);
  prim11b = create_line(x+75, y, x, y+75, 0x0F);
  prim11c = create_line(x, y+75, x-75, y, 0x3C);
  prim11d = create_line(x-75, y, x, y-75, 0x03);
}
