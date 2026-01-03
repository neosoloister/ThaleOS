#include "../cpu/idt.h"
#include "../cpu/isr.h"
#include "../cpu/timer.h"
#include "../driver/keyboard.h"
#include "../driver/vga.h"
#include "../lib/kprintf.h"
#include "shell.h"
#include <stdint.h>

void kernel_main(void) {
    vga_clear();
    idt_init();
    isr_install();
    init_keyboard();
    init_timer(1000);
    __asm__ __volatile__("sti");

    kprintf("ThaleOS Kernel initialized.\n");

    shell_init();
}