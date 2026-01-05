#include "../cpu/idt.h"
#include "../cpu/isr.h"
#include "../cpu/timer.h"
#include "../driver/keyboard.h"
#include "../driver/vga.h"
#include "../lib/kprintf.h"
#include "../lib/mem.h"

#include "shell.h"
#include "../fs/fat.h"
#include <stdint.h>
#include <string.h>


void kernel_init () {
    vga_clear();
    idt_init();
    isr_install();
    init_keyboard();
    init_timer(1000);
    init_mem();

    void *ptr = malloc(512);
    
    if (ptr == NULL) {
        kprintf("Memory allocation failed!\n");
    } else {
        kprintf("Memory allocation successful at %p\n", ptr);
        free(ptr);
    }

    fat_init();
    fat_list_root();

    __asm__ __volatile__("sti");
    kprintf("ThaleOS Kernel initialized.\n");
}

void kernel_main(void) {
    kernel_init();
    vga_clear();
    shell_init();
}