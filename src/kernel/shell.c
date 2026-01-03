#include "shell.h"
#include "../cpu/ports.h" // for shutdown/hlt
#include "../driver/keyboard.h"
#include "../driver/vga.h"
#include "../lib/kprintf.h"
#include <string.h>
#include "../driver/rtc.h"

void cmd_help() {
    kprintf("Available commands:\n");
    kprintf("  help     - Show this list\n");
    kprintf("  clear    - Clear the screen\n");
    kprintf("  time     - Show current uptime ticks\n");
    kprintf("  shutdown - Halt the CPU\n");
    kprintf("  reboot   - Reboot the system\n");
    kprintf("  echo     - Echo the arguments\n");
    kprintf("  date     - Show current RTC time\n");
}

void cmd_shutdown() {
    kprintf("Shutting down...\n");
    port_word_out(0x604, 0x2000); // Shutdown
    __asm__ __volatile__("cli; hlt");
}

void cmd_reboot() {
    kprintf("Rebooting...\n");
    uint8_t good = 0x02;
    while (good & 0x02)
        good = port_byte_in(0x64);
    port_byte_out(0x64, 0xfe);
}

void cmd_time() {
    extern volatile uint32_t tick;
    kprintf("Uptime: %d ticks (approx %d seconds)\n", tick, tick / 1000);
}

void cmd_date() {
    rtc_time_t t;
    rtc_get_time(&t);
    kprintf("Current time (UTC): 20");
    if (t.year < 10) kprintf("0");
    kprintf("%d-", t.year);
    if (t.month < 10) kprintf("0");
    kprintf("%d-", t.month);
    if (t.day < 10) kprintf("0");
    kprintf("%d ", t.day);
    if (t.hour < 10) kprintf("0");
    kprintf("%d:", t.hour);
    if (t.minute < 10) kprintf("0");
    kprintf("%d:", t.minute);
    if (t.second < 10) kprintf("0");
    kprintf("%d\n", t.second);
}

void cmd_echo(char *args) {
    kprintf("%s\n", args);
}

void shell_exec(char *input_buffer) {
    if (strcmp(input_buffer, "help") == 0) {
        cmd_help();
    } 
    else if (strcmp(input_buffer, "clear") == 0) {
        vga_clear();
    } 
    else if (strcmp(input_buffer, "shutdown") == 0) {
        cmd_shutdown();
    }
    else if (strcmp(input_buffer, "reboot") == 0) {
        cmd_reboot();
    }
    else if (strcmp(input_buffer, "time") == 0) {
        cmd_time();
    } 
    else if (strcmp(input_buffer, "date") == 0) {
        cmd_date();
    } 
    else if (strncmp(input_buffer, "echo ", 5) == 0) {
        cmd_echo(input_buffer + 5);
    }
    else if (strcmp(input_buffer, "echo") == 0) {
        kprintf("\n");
    }
    else if (strlen(input_buffer) > 0) {
        kprintf("Unknown command: %s\n", input_buffer);
    }
}

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
        if (strlen(input_buffer) > 0) {
            shell_exec(input_buffer);
        }
    }
}