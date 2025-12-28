#include <stdint.h>

static volatile uint16_t* const VGA = (uint16_t*)0xB8000;

void kernel_main(void) {
    VGA[0] = (0x1F << 8) | 'K';
    VGA[1] = (0x1F << 8) | 'E';
    VGA[2] = (0x1F << 8) | 'R';
    VGA[3] = (0x1F << 8) | 'N';
    VGA[4] = (0x1F << 8) | 'E';
    VGA[5] = (0x1F << 8) | 'L';
    for (;;) __asm__ volatile("hlt");
}