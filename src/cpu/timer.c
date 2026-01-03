#include "timer.h"
#include "isr.h"
#include "ports.h"

#include "../driver/rtc.h"
#include "../driver/vga.h"
#include "../lib/kprintf.h"

volatile uint32_t tick = 0;

static void print_time_status() {
    uint8_t old_row = vga_get_cursor_row();
    uint8_t old_col = vga_get_cursor_column();

    vga_set_cursor(0, 60); // Top right-ish
    
    rtc_time_t t;
    rtc_get_time(&t);
    
    // Simple kprintf usage. Note: kprintf uses vga_write/putc which advances cursor.
    // We rely on restoring cursor afterwards.
    kprintf("20%d%d-%d%d-%d%d %d%d:%d%d:%d%d", 
        t.year / 10, t.year % 10,
        t.month / 10, t.month % 10,
        t.day / 10, t.day % 10,
        t.hour / 10, t.hour % 10,
        t.minute / 10, t.minute % 10,
        t.second / 10, t.second % 10);

    vga_set_cursor(old_row, old_col);
}

static void timer_callback(registers_t *regs) {
    tick++;
    if (tick % 1000 == 0) {
        print_time_status();
    }
    (void)regs;
}

void init_timer(uint32_t freq) {
    register_interrupt_handler(IRQ0, timer_callback);

    uint32_t divisor = 1193180 / freq;
    uint8_t low = (uint8_t)(divisor & 0xFF);
    uint8_t high = (uint8_t)((divisor >> 8) & 0xFF);

    port_byte_out(0x43, 0x36);
    port_byte_out(0x40, low);
    port_byte_out(0x40, high);
}

void sleep(uint32_t ticks) {
    uint32_t eticks = tick + ticks;
    while (tick < eticks);
}