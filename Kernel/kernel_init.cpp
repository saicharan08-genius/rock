#include <port.h>
#include <shitio.h>
#include <interrupt.h>
#include <keyboard.h>
#include <paging.h>
#include <process.h>
#include <stdint.h>
#include <graphics.h>

extern void load_gdt(void) asm("load_gdt");
extern void div_test(void) asm("test_div");

using namespace standardout;
using namespace MM;

extern "C" void kernel_main(void)
{
    load_gdt();
    initalize(VGA_WHITE, VGA_BLUE);
    t_print("\nKernel Debug");
    page_setup();
    idt_init();
    page_frame_init(0xF42400); //Reserves ~ 16mb

    asm volatile("sti");

    sprit_draw_main();
    clear_screen();

    /* tests processes allocation */

    s_print(VGA_LIGHT_BLUE, 50, 1, "crepOS Dynamic Debugger");

    block_show();
    grab_current_y();
    draw_vline(VGA_MAGENTA, 48, 0, 25);

    process proc(0x2001);
    proc.pmalloc(0x4);
    uint16_t *ptrbruh = (uint16_t*)proc.pmalloc(0x8);
    proc.pfree(ptrbruh);
    uint16_t *ptruh = (uint16_t*)proc.pmalloc(0x4);

    k_print("> ");
    startInput();

    for(;;);
}
