// agon_crayon.ino - Test of Agon 64-color mode
//
// This program shows a VGA display of 800x600 pixels at 40 MHz, with 64 colors.
// It makes use of some peripheral-related code concepts/constructs from FabGL.
//
// V0.1

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

extern int32_t g_hscroll;
extern int32_t g_vscroll; 


/*
  Buffer arrangement:

  descr#0 -> buffer#0 (line#0, line#1)
  descr#1 -> buffer#1 (line#2, line#3)
  descr#2 -> buffer#2 (line#4, line#5)
  descr#3 -> buffer#3 (line#6, line#7)

  descr#4 -> buffer#0 (line#8, line#9)
  descr#5 -> buffer#1 (line#10, line#11)
  descr#6 -> buffer#2 (line#12, line#13)
  descr#7 -> buffer#3 (line#14, line#15)
  ...
  descr#296 -> buffer#0 (line#592, line#593)
  descr#297 -> buffer#1 (line#594, line#595)
  descr#298 -> buffer#2 (line#596, line#597)
  descr#299 -> buffer#3 (line#598, line#599)
  ...
  vertical blanking descriptors here
*/

/*void IRAM_ATTR vsync_irq_handler(void* param) {
  lldesc_t volatile * descr_addr = (lldesc_t volatile *)I2S1.out_eof_des_addr;
  uint32_t descr_index = descr_addr - dma_descriptor;
  uint32_t buffer_index = descr_index + NUM_ACTIVE_BUFFERS;
  if (buffer_index >= ACT_LINES/NUM_LINES_PER_BUFFER) {
    buffer_index -= ACT_LINES/NUM_LINES_PER_BUFFER;
  }
  g_video_buffer[buffer_index & (NUM_ACTIVE_BUFFERS - 1)].paint(buffer_index * NUM_LINES_PER_BUFFER);
  I2S1.int_clr.val = I2S1.int_st.val;
}*/

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

  g_video_buffer[0].paint(0); // just to prevent black screen during debugging

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
  uint32_t prev_descr = (uint32_t) dma_descriptor;
  bool eof = false;
  while (true) {
    uint32_t descr_addr = (uint32_t) I2S1.out_link_dscr;//out_eof_des_addr;
    uint32_t descr_index = (descr_addr - (uint32_t) dma_descriptor) / sizeof(lldesc_t);
    if (descr_index < ACT_LINES/NUM_LINES_PER_BUFFER) {
      eof = false;
      /*g_video_buffer[0].paint(100);
      g_video_buffer[1].paint(205);
      g_video_buffer[2].paint(311);
      g_video_buffer[3].paint(467);
      g_video_buffer[4].paint(111);
      g_video_buffer[5].paint(213);
      g_video_buffer[6].paint(317);
      g_video_buffer[7].paint(443);*/
      //g_video_buffer[descr_index & (NUM_ACTIVE_BUFFERS-1)].paint(5);
      g_video_buffer[descr_index & (NUM_ACTIVE_BUFFERS-1)].paint(descr_index*NUM_LINES_PER_BUFFER);
      //g_video_buffer[descr_index].paint(descr_index * NUM_LINES_PER_BUFFER);
    } else if (!eof) {
      // End of frame
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
      g_hscroll += scroll_dx[scroll_mode];
      g_vscroll += scroll_dy[scroll_mode];
   }
    /*
    g_video_buffer[descr_index & (NUM_ACTIVE_BUFFERS - 1)].paint(descr_index * NUM_LINES_PER_BUFFER);
    if (descr_addr != prev_descr) {
      prev_descr = descr_addr;
      if (descr_index <= ACT_LINES/NUM_LINES_PER_BUFFER) {
        uint32_t buffer_index = descr_index + NUM_ACTIVE_BUFFERS;
        if (buffer_index >= ACT_LINES/NUM_LINES_PER_BUFFER) {
          buffer_index -= ACT_LINES/NUM_LINES_PER_BUFFER;
        }
        g_video_buffer[buffer_index & (NUM_ACTIVE_BUFFERS - 1)].paint(buffer_index * NUM_LINES_PER_BUFFER);
      }
    }
    */
  }
}