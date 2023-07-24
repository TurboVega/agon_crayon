// di_manager.h - Function declarations for managing drawing-instruction primitives
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

#include <vector>
#include "di_primitive.h"

class DiManager {
    public:
    DiManager();
    ~DiManager();

    DiPrimitive* create_point(int32_t x, int32_t y, uint8_t color);
    DiPrimitive* create_line(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint8_t color);
    DiPrimitive* create_solid_rectangle(int32_t x, int32_t y, uint32_t width, uint32_t height, uint8_t color);
    DiPrimitive* create_triangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint8_t color);

    void IRAM_ATTR run(uint32_t dma_descr_array, uint32_t size_of_descr, uint8_t** dma_buffers);
    void IRAM_ATTR task_body();
    void stop();
    void clear();
    void IRAM_ATTR draw_primitives(DiPaintParams* params);

    protected:
    uint32_t  m_dma_descr_array;
    uint32_t  m_size_of_descr;
    uint8_t** m_dma_buffers;
    std::vector<DiPrimitive*> m_groups[NUM_VERTICAL_GROUPS];

    void add_primitive(DiPrimitive* prim);
    void IRAM_ATTR on_vertical_blank();
};
