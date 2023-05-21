// agon_crayon.ino - Test of Agon 64-color mode
//
// This program shows a VGA display of 800x600 pixels at 40 MHz, with 64 colors.
// It makes use of some peripheral-related code concepts/constructs from FabGL.
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

#include <stdint.h>
#include <stddef.h>
#include "soc/i2s_struct.h"
#include "soc/i2s_reg.h"
#include "driver/periph_ctrl.h"
#include "soc/rtc.h"
#include "driver/gpio.h"
#include "soc/io_mux_reg.h"
#include "rom/lldesc.h"
#include "di_video_buffer.h"
#include "fabgl_pieces.h"

DMA_ATTR lldesc_t volatile dma_descriptor[DMA_TOTAL_DESCR];

DMA_ATTR DiVideoBuffer g_video_buffer[NUM_ACTIVE_BUFFERS];
DMA_ATTR DiVideoScanLine g_front_porch;
DMA_ATTR DiVideoBuffer g_vertical_sync;
DMA_ATTR DiVideoScanLine g_back_porch;

intr_handle_t isr_handle;

DiPaintParams g_params;

void init_dma_descriptor(DiVideoScanLine* vbuf, uint32_t descr_index) {
  lldesc_t volatile * dd = &dma_descriptor[descr_index];

  if (descr_index == 0) {
    dma_descriptor[DMA_TOTAL_DESCR - 1].qe.stqe_next = (lldesc_t*)dd;
  } else {
    dma_descriptor[descr_index - 1].qe.stqe_next = (lldesc_t*)dd;
  }

  dd->sosf = dd->offset = dd->eof = 0;
  dd->owner = 1;
  dd->size = vbuf->get_buffer_size();
  dd->length = vbuf->get_buffer_size();
  dd->buf = (uint8_t volatile *)vbuf->get_buffer_ptr();
}

void init_dma_descriptor(DiVideoBuffer* vbuf, uint32_t descr_index) {
  lldesc_t volatile * dd = &dma_descriptor[descr_index];

  if (descr_index == 0) {
    dma_descriptor[DMA_TOTAL_DESCR - 1].qe.stqe_next = (lldesc_t*)dd;
  } else {
    dma_descriptor[descr_index - 1].qe.stqe_next = (lldesc_t*)dd;
  }

  dd->sosf = dd->offset = dd->eof = 0;
  dd->owner = 1;
  dd->size = vbuf->get_buffer_size();
  dd->length = vbuf->get_buffer_size();
  dd->buf = (uint8_t volatile *)vbuf->get_buffer_ptr();
}

extern void init_stars();

void setup() {
	disableCore0WDT(); delay(200); // Disable the watchdog timer on CPU core 0
	disableCore1WDT(); delay(200); // Disable the watchdog timer on CPU core 1
  
  init_stars();
  
  // DMA buffer chain: ACT
  uint32_t descr_index = 0;
  for (uint32_t i = 0; i < NUM_ACTIVE_BUFFERS; i++) {
    g_video_buffer[i].init_to_black();
  }
  for (uint32_t i = 0; i < ACT_LINES/NUM_LINES_PER_BUFFER; i++) {
    init_dma_descriptor(&g_video_buffer[i & (NUM_ACTIVE_BUFFERS - 1)], descr_index);
    dma_descriptor[descr_index++].eof = 1;
  }

  // DMA buffer chain: VFP
  g_front_porch.init_to_black();
  for (uint i = 0; i < VFP_LINES; i++) {
    init_dma_descriptor(&g_front_porch, descr_index++);
  }

  // DMA buffer chain: VS
  g_vertical_sync.init_for_vsync();
  for (uint i = 0; i < VS_LINES/NUM_LINES_PER_BUFFER; i++) {
    init_dma_descriptor(&g_vertical_sync, descr_index++);
  }
  
  // DMA buffer chain: VBP
  g_back_porch.init_to_black();
  for (uint i = 0; i < VBP_LINES; i++) {
    init_dma_descriptor(&g_back_porch, descr_index++);
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
  I2S1.conf2.lcd_tx_wrx2_en = 0; // NOT 1
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
  I2S1.lc_conf.val    = I2S_OUT_DATA_BURST_EN | I2S_OUTDSCR_BURST_EN;
  I2S1.out_link.addr  = (uint32_t)dma_descriptor;

  //esp_intr_alloc(ETS_I2S1_INTR_SOURCE, ESP_INTR_FLAG_LEVEL1,
  //              vsync_irq_handler, NULL, &isr_handle);
  I2S1.int_clr.val = 0xFFFFFFFF;
  //I2S1.int_ena.out_eof = 1;

  I2S1.out_link.start = 1;
  I2S1.conf.tx_start  = 1;
}

// dummy data for testing only
extern uint32_t sdx[10];
extern uint32_t sdy[10];
int8_t xdir[10] = {1,-1,1,-1,1,-1,1,-1,1,-1};
int8_t ydir[10] = {-1,1,-1,1,-1,1,-1,1,-1,1};
uint8_t scroll_mode = 0;
int32_t scroll_dx[5] = {0,1,-1,1,-1};
int32_t scroll_dy[5] = {0,1,0,-1,0};
uint8_t scroll_count = 0;

IRAM_ATTR void loop() {
  bool eof = false;
  g_params.m_screen_width = ACT_PIXELS;
  g_params.m_screen_height = ACT_LINES;
  while (true) {
    uint32_t descr_addr = (uint32_t) I2S1.out_link_dscr;;
    uint32_t descr_index = (descr_addr - (uint32_t) dma_descriptor) / sizeof(lldesc_t);
    if (descr_index < ACT_LINES/NUM_LINES_PER_BUFFER) {
      // Active scan line (not end of frame)
      eof = false;
      g_params.m_line_index = descr_index * NUM_LINES_PER_BUFFER;
      g_video_buffer[descr_index & (NUM_ACTIVE_BUFFERS-1)].paint(&g_params);
    } else if (!eof) {
      // End of frame (vertical blanking area)
      eof = true;

      // move diamonds
      for (int d = 0; d < 10; d++) {
        if (xdir[d] > 0) {
          if (++sdx[d] >= 710) {
            xdir[d] = -1;
          }
        } else {
          if (--sdx[d] <= 90) {
            xdir[d] = 1;
          }
        }

        if (ydir[d] > 0) {
          if (++sdy[d] >= 510) {
            ydir[d] = -1;
          }
        } else {
          if (--sdy[d] <= 90) {
            ydir[d] = 1;
          }
        }
      }

      // do scrolling
      if (++scroll_count >= 30) {
        scroll_count = 0;
        if (++scroll_mode >= 5) {
          scroll_mode = 0;
        }
      }
      g_params.m_horiz_scroll += scroll_dx[scroll_mode];
      g_params.m_vert_scroll += scroll_dy[scroll_mode];
   }
  }
}