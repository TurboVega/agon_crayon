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

TaskHandle_t g_otfTaskHandle;

/*
The on_the_fly task is responsible for managing the few scan line buffers used to
output data (via DMA) to the VGA port. Once started, this task runs continuously
until it is stopped by a video mode change (if one ever happens).
*/
void on_the_fly(void* param)
{
    ((DiManager*)param)->task_body();
}

DiManager::DiManager() {
}

DiManager::~DiManager() {
    for (int g = 0; g < NUM_VERTICAL_GROUPS; g++) {
        std::vector<DiPrimitive*> * vp = &m_groups[g];
        for (auto prim = vp->begin(); prim != vp->end(); ++prim) {
            delete *prim;
        }
    }
}

void DiManager::clear() {
    // NOTE: need to delete each prim only once!!
    for (int g = 0; g < NUM_VERTICAL_GROUPS; g++) {
        std::vector<DiPrimitive*> * vp = &m_groups[g];
        for (auto prim = vp->begin(); prim != vp->end(); ++prim) {
            delete *prim;
        }
        vp->clear();
    }
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
                prim = new DiDiagonalRightLine(x1, y1, x2 - x1, color);
            } else {
                prim = new DiGeneralLine(x1, y1, x2, y2, color);
            }
        } else {
            if (y2 - y1 == x2 - x1) {
                prim = new DiDiagonalLeftLine(x2, y1, x2 - x1, color);
            } else {
                prim = new DiGeneralLine(x1, y1, x2, y2, color);
            }
        }
    } else {
        if (y1 == y2) {
            prim = new DiHorizontalLine(x1, y1, x1 - x2 + 1, color);
        } else if (y1 < y2) {
            if (y2 - y1 == x1 - x2) {
                prim = new DiDiagonalRightLine(x2, y1, x1 - x2, color);
            } else {
                prim = new DiGeneralLine(x1, y1, x2, y2, color);
            }
        } else {
            if (y2 - y1 == x1 - x2) {
                prim = new DiDiagonalLeftLine(x1, y1, x1 - x2, color);
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

void IRAM_ATTR DiManager::run(uint32_t dma_descr_array, uint32_t size_of_descr, uint8_t** dma_buffers) {
  m_dma_descr_array = dma_descr_array;
  m_size_of_descr = size_of_descr;
  m_dma_buffers = dma_buffers;

  xTaskCreatePinnedToCore(on_the_fly, // Task function
    "OnTheFly", // Task name
    2048,		// Stack size
    this,		// Parameter
    (configMAX_PRIORITIES - 1),	// Priority, the highest
    &g_otfTaskHandle, // Save task handle
    0           // Pin this task to ESP32 Core 0
	);
}

void DiManager::stop() {
    if (g_otfTaskHandle)
    {
      vTaskDelete(g_otfTaskHandle);
      g_otfTaskHandle = NULL;
    }
    clear();
}

void IRAM_ATTR DiManager::draw_primitives(DiPaintParams* params) {
    int32_t g = params->m_line_index >> VERTICAL_GROUP_INDEX_SHIFT;
    std::vector<DiPrimitive*> * vp = &m_groups[g];
    for (auto prim = vp->begin(); prim != vp->end(); ++prim) {
        (*prim)->paint(params);
    }
}

void IRAM_ATTR DiManager::on_vertical_blank() {
}

extern "C" {
extern uint32_t int_ena_bits0;
extern uint32_t int_ena_bits1;
}

void IRAM_ATTR DiManager::task_body() {
  int_ena_bits0 = I2S1.int_ena.val;
  disableCore0WDT(); delay(200);
  disableCore1WDT(); delay(200);
  //portDISABLE_INTERRUPTS(); // disabling them prevents display from showing

  //DiPrimitive* prim1 = create_solid_rectangle(0, 0, 800, 600, 0x00);
  DiPrimitive* prim1 = create_solid_rectangle(20, 0, 760, 600, 0x05);

  DiPrimitive* prim2 = create_line(0, 19, 799, 19, 0x33); // horiz

  DiPrimitive* prim3a = create_line(1, 1, 1, 598, 0x3F); // vert
  DiPrimitive* prim3b = create_line(2, 2, 2, 597, 0x3F); // vert
  DiPrimitive* prim3c = create_line(3, 3, 3, 596, 0x3F); // vert
  DiPrimitive* prim3d = create_line(4, 4, 4, 595, 0x3F); // vert

  DiPrimitive* prim4 = create_line(0, 599, 799, 599, 0x03);
  DiPrimitive* prim5 = create_line(799, 1, 799, 598, 0x0A);

  DiPrimitive* prim6 = create_line(50, 13, 75, 17, 0x1E);
  DiPrimitive* prim7 = create_line(750, 431, 786, 411, 0x1E);
  DiPrimitive* prim8 = create_solid_rectangle(150, 300, 227, 227, 0x20);

  DiPrimitive* prim10 = create_triangle(450, 330, 520, 402, 417, 375, 0x15);
  //DiPrimitive* prim10a = create_point(450, 330, 0x30);
  //DiPrimitive* prim10b = create_point(520, 402, 0x0C);
  //DiPrimitive* prim10c = create_point(417, 375, 0x03);

  DiPrimitive* prim10d = create_line(450, 330, 520, 402, 0x30);
  DiPrimitive* prim10e = create_line(520, 402, 417, 375, 0x0C);
  DiPrimitive* prim10f = create_line(417, 375, 450, 330, 0x03);

  /*for (int i = 0; i < 32; i++) {
    create_line(i*20, 2, i*20+14, 2, 0x33);
    if (int_ena_bits & (1<<(31-i)) {
      create_line(i*20, 5, i*20+14, 5, 0x18);
    }
  }*/

  DiPaintParams paint_params;
  paint_params.m_horiz_scroll = 0;
  paint_params.m_vert_scroll = 0;
  paint_params.m_screen_width = ACT_PIXELS;
  paint_params.m_screen_height = ACT_LINES;

  uint32_t current_line_index = ACT_LINES;
  uint32_t current_buffer_index = 0;
  bool end_of_frame = false;
  uint8_t inc = 0;
  uint32_t prior_index = 0xFFFFFFFF;

  while (true) {
    uint32_t descr_addr = (uint32_t) I2S1.out_link_dscr;
    uint32_t descr_index = (descr_addr - m_dma_descr_array) / m_size_of_descr;
    if (descr_index < (ACT_LINES*2)) {
      uint32_t dma_line_index = ((descr_index + 1) >> 1);
      if (dma_line_index == prior_index) continue;
      prior_index = dma_line_index;

      uint32_t dma_buffer_index = dma_line_index & (NUM_ACTIVE_BUFFERS-1);

      // Draw enough lines to stay ahead of DMA.
      while (current_line_index < ACT_LINES && current_buffer_index != dma_buffer_index) {
        paint_params.m_line8 = m_dma_buffers[current_buffer_index];
        paint_params.m_line32 = (uint32_t*)paint_params.m_line8;
        paint_params.m_line_index = current_line_index;
        paint_params.m_scrolled_y = current_line_index + paint_params.m_vert_scroll;
        //draw_primitives(&paint_params);
        //memset(paint_params.m_line8,(current_line_index+inc)&0x3F,800);
        //memset(paint_params.m_line8,0x00,780);

        prim1->paint(&paint_params);
        if (current_line_index==19) prim2->paint(&paint_params);

        prim3a->paint(&paint_params);
        prim3b->paint(&paint_params);
        prim3c->paint(&paint_params);
        prim3d->paint(&paint_params);

        //prim4->paint(&paint_params);
        //prim5->paint(&paint_params);
        prim6->paint(&paint_params);
        prim7->paint(&paint_params);
        if (current_line_index >= 300 && current_line_index < 527) prim8->paint(&paint_params);

        prim10->paint(&paint_params);
        //prim10a->paint(&paint_params);
        //prim10b->paint(&paint_params);
        //prim10c->paint(&paint_params);
        prim10d->paint(&paint_params);
        prim10e->paint(&paint_params);
        prim10f->paint(&paint_params);

        ++current_line_index;
        if (++current_buffer_index >= NUM_ACTIVE_BUFFERS) {
          current_buffer_index = 0;
        }
      }
      end_of_frame = false;
    } else if (!end_of_frame) {
      ++inc;
      // Handle modifying primitives before the next frame starts.
      on_vertical_blank();

      // Prepare the start of the next frame.
      for (current_line_index = 0;
            current_line_index < NUM_ACTIVE_BUFFERS;
            current_line_index++) {
        paint_params.m_line8 = m_dma_buffers[current_line_index];
        paint_params.m_line32 = (uint32_t*)paint_params.m_line8;
        paint_params.m_line_index = current_line_index;
        paint_params.m_scrolled_y = current_line_index + paint_params.m_vert_scroll;
        //draw_primitives(&paint_params);
        //memset(paint_params.m_line8,(current_line_index+inc)&0x3F,800);
        memset(paint_params.m_line8,0x00,800);
        //prim1->paint(&paint_params);

        prim3a->paint(&paint_params);
        prim3b->paint(&paint_params);
        prim3c->paint(&paint_params);
        prim3d->paint(&paint_params);

        //prim4->paint(&paint_params);
        //prim5->paint(&paint_params);
        prim6->paint(&paint_params);

        /*if (current_line_index == 2) {
              for (int i = 0; i < 32; i++) {
                memset(paint_params.m_line8+i*20, 0x33, 12);
            }
        }
        if (current_line_index == 5) {
              for (int i = 0; i < 32; i++) {
                if (int_ena_bits0 & (1<<(31-i))) {
                    memset(paint_params.m_line8+i*20, 0x18, 12);
                }
            }
        }
        if (current_line_index == 7) {
              for (int i = 0; i < 32; i++) {
                if (int_ena_bits1 & (1<<(31-i))) {
                    memset(paint_params.m_line8+i*20, 0x18, 12);
                }
            }
        }*/
      }
      end_of_frame = true;
      current_line_index = NUM_ACTIVE_BUFFERS;
      current_buffer_index = 0;
      prior_index = 0xFFFFFFFF;
    }
  }
}
