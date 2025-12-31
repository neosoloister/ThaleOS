#include <stdint.h>
#include "../driver/vga.h"
#include "../lib/kprintf.h"

void kernel_main(void) {
    vga_clear();
    vga_set_attr(VGA_YELLOW, VGA_BLUE);
    int x[3] = {10, 20, 30};
    for (int i = 0; i < 3; i++) {
        kprintf("x = %d at %p\n", x[i], x[i]);
    }
}