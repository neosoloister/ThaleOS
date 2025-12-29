#include <stdint.h>
#include "../driver/vga.h"
#include "../lib/kprintf.h"

void kernel_main(void) {
    vga_clear();
    vga_set_attr(VGA_YELLOW, VGA_BLUE);
    vga_write("Hello from Kernel!\n");
}