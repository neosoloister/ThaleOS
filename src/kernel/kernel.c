#include <stdint.h>
#include "../driver/vga.h"
#include "../lib/kprintf.h"
#include "../cpu/idt.h"
#include "../cpu/isr.h"
#include "../driver/keyboard.h"

void kernel_main(void) {
    idt_init();
    isr_install();
    init_keyboard();
    __asm__ __volatile__("sti");

    vga_clear();
    vga_fill(VGA_YELLOW, VGA_BLUE);
    vga_set_attr(VGA_YELLOW, VGA_BLUE);
    
    kprintf("Testing interrupts...\n");
    __asm__ __volatile__("int $0");
    kprintf("Back from interrupt!\n");

    kprintf("Type something!\n");
    
    while (1) {
        char c = keyboard_getc();
        if (c != 0) {
            kprintf("%c", c);
        }
    }
}