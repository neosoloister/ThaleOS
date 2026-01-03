#include "../cpu/idt.h"
#include "../cpu/isr.h"
#include "../cpu/timer.h"
#include "../driver/keyboard.h"
#include "../driver/vga.h"
#include "../lib/kprintf.h"
#include <stdint.h>

void kernel_main(void) {
  idt_init();
  isr_install();
  init_keyboard();
  init_timer(1000);
  __asm__ __volatile__("sti");
  vga_clear();
  vga_fill(VGA_YELLOW, VGA_BLUE);
  vga_set_attr(VGA_YELLOW, VGA_BLUE);

  kprintf("Testing interrupts...\n");
  __asm__ __volatile__("int $0");
  kprintf("Back from interrupt!\n");
  int count_sec = 0;
  while (1) {
    vga_set_cursor(6, 0);
    sleep(1000);
    kprintf("%d second passed\n", count_sec++);
  }
}