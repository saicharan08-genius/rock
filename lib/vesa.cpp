#include <kernel/mm/virtualPageManager.h>
#include <kernel/stivale.h>
#include <lib/font.h>
#include <lib/vesa.h>

namespace kernel {

void vesa::setPixel(uint16_t x, uint16_t y, uint32_t colour) {
    *(volatile uint32_t*)(((uint64_t)framebuffer + HIGH_VMA) + ((y * pitch) + (x * bpp / 8))) = colour; 
}

uint32_t vesa::grabColour(uint16_t x, uint16_t y) {
    return *(volatile uint32_t*)(((uint64_t)framebuffer + HIGH_VMA) + ((y * pitch) + (x * bpp / 8)));
}

void vesa::initVESA(stivaleInfo_t *stivale) {
    framebuffer = stivale->framebufferAddr; 
    height = stivale->framebufferHeight;
    width = stivale->framebufferWidth;
    pitch = stivale->framebufferPitch;
    bpp = stivale->framebufferBpp;
}

void vesa::renderChar(uint64_t x, uint64_t y, uint32_t fg, char c) {
    for(uint8_t i = 0; i < 8; i++) {
        for(uint8_t j = 0; j < 8; j++) {
            if((font[(uint8_t)c][i] >> j) & 1) {
                setPixel(j + x, y + i, fg);
            }
        }
    }
}

}