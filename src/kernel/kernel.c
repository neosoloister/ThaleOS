#include <stdint.h>
#include "../driver/vga.h"

void kernel_main(void) {
    vga_clear();
    vga_write("Hello from Kernel!");
}