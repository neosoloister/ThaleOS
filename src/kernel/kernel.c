#include "../cpu/idt.h"
#include "../cpu/isr.h"
#include "../cpu/timer.h"
#include "../driver/keyboard.h"
#include "../driver/vga.h"
#include "../lib/kprintf.h"
#include "../lib/mem.h"
#include "shell.h"
#include <stdint.h>
#include <string.h>

void kernel_main(void) {
    vga_clear();
    idt_init();
    isr_install();
    init_keyboard();
    init_timer(1000);
    init_mem();

    void *ptr = malloc(4 * 1024 * 1024);
    
    if (ptr == NULL) {
        kprintf("Memory allocation failed!\n");
    } else {
        kprintf("Memory allocation successful at %p\n", ptr);
    }
    void *test = malloc(32 * 1024 * 1024);
    if (test == NULL) {
        kprintf("Memory allocation failed!\n");
    } else {
        kprintf("Memory allocation successful at %p\n", ptr);
    }

    __asm__ __volatile__("sti");

    kprintf("ThaleOS Kernel initialized.\n");

    mem_dump();
    shell_init();
}