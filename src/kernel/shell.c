#include "shell.h"
#include "../cpu/ports.h" // for shutdown/hlt
#include "../driver/keyboard.h"
#include "../driver/vga.h"
#include "../lib/kprintf.h"
#include <string.h>
void shell_init() {
    kprintf("Welcome to ThaleOS Shell!\n");
    kprintf("Type 'help' for commands.\n");

    char input_buffer[256];
    int index = 0;

    while (1) {
        kprintf("ThaleOS> ");

        index = 0;
        while (1) {
            char c = keyboard_getc();
            if (c == 0)
                continue;

            if (c == '\n') {
                input_buffer[index] = '\0';
                kprintf("\n");
                break;
            } else if (c == '\b') {
                if (index > 0) {
                    index--;
                    kprintf("\b \b");
                }
            } else {
                if (index < 255) {
                    input_buffer[index++] = c;
                    kprintf("%c", c);
                }
            }
        }

        // Process command
        if (strcmp(input_buffer, "help") == 0) {
            kprintf("Available commands:\n");
            kprintf("  help     - Show this list\n");
            kprintf("  clear    - Clear the screen\n");
            kprintf("  time     - Show current uptime ticks\n");
            kprintf("  shutdown - Halt the CPU\n");
            kprintf("  reboot   - Reboot the system\n");
        } 
        else if (strcmp(input_buffer, "clear") == 0) {
            vga_clear();
        } 
        else if (strcmp(input_buffer, "shutdown") == 0) {
            kprintf("Shutting down...\n");
            port_word_out(0x604, 0x2000); // Shutdown
            __asm__ __volatile__("cli; hlt");
        }
        else if (strcmp(input_buffer, "reboot") == 0) {
            kprintf("Rebooting...\n");
            uint8_t good = 0x02;
            while (good & 0x02)
                good = port_byte_in(0x64);
            port_byte_out(0x64, 0xfe);
        }
        else if (strcmp(input_buffer, "time") == 0) {
            extern volatile uint32_t tick;
            kprintf("Uptime: %d ticks (approx %d seconds)\n", tick, tick / 1000);
        } 
        else if (strlen(input_buffer) > 0) {
            kprintf("Unknown command: %s\n", input_buffer);
        }
    }
}
