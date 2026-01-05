#include "shell.h"
#include "../cpu/ports.h" // for shutdown/hlt
#include "../driver/keyboard.h"
#include "../driver/vga.h"
#include "../lib/kprintf.h"
#include <string.h>
#include "../driver/rtc.h"

#include <stddef.h>
#include "../fs/fat.h"
#include "../lib/mem.h"

void cmd_help() {
    kprintf("Available commands:\n");
    kprintf("  help            - Show this list\n");
    kprintf("  clear           - Clear the screen\n");
    kprintf("  time            - Show current uptime ticks\n");
    kprintf("  shutdown        - Halt the CPU\n");
    kprintf("  reboot          - Reboot the system\n");
    kprintf("  echo <msg>      - Echo the arguments\n");
    kprintf("  date            - Show current RTC time\n");
    kprintf("  ls              - List files in root directory\n");
    kprintf("  touch <file>    - Create a new file\n");
    kprintf("  rm <file>       - Delete a file\n");
    kprintf("  mv <old> <new>  - Rename a file\n");
    kprintf("  cp <src> <dest> - Copy a file\n");
    kprintf("  cat <file>      - Read a file\n");
    kprintf("  write <f> <txt> - Write text to file\n");
    kprintf("  mem_dump         - Dump all of memories\n");
}

/* ... existing commands ... */
void cmd_memdump() {
    mem_dump();
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

// Basic argument splitter: replaces first space with null and points to next char
static char* split_args(char *args) {
    char *p = args;
    while (*p) {
        if (*p == ' ') {
            *p = 0;
            return p + 1;
        }
        p++;
    }
    return NULL;
}

void cmd_ls() {
    fat_list_root();
}

void cmd_touch(char *args) {
    if (!args || strlen(args) == 0) {
        kprintf("Usage: touch <filename>\n");
        return;
    }
    if (fat_create(args) == 0) {
        kprintf("Created file: %s\n", args);
    }
}

void cmd_rm(char *args) {
    if (!args || strlen(args) == 0) {
        kprintf("Usage: rm <filename>\n");
        return;
    }
    if (fat_delete(args) == 0) {
        kprintf("Deleted file: %s\n", args);
    }
}

void cmd_mv(char *args) {
    char *dest = split_args(args);
    if (!dest) {
        kprintf("Usage: mv <old_name> <new_name>\n");
        return;
    }
    if (fat_rename(args, dest) == 0) {
        kprintf("Renamed %s to %s\n", args, dest);
    }
}

void cmd_cp(char *args) {
    char *dest = split_args(args);
    if (!dest) {
        kprintf("Usage: cp <src> <dest>\n");
        return;
    }
    if (fat_copy(args, dest) == 0) {
        kprintf("Copied %s to %s\n", args, dest);
    }
}

void cmd_cat(char *args) {
    if (!args || strlen(args) == 0) {
        kprintf("Usage: cat <filename>\n");
        return;
    }
    
    // Allocate buffer for reading (max 512 bytes for now)
    uint8_t *buf = malloc(512);
    if (!buf) {
        kprintf("Memory allocation failed.\n");
        return;
    }
    
    if (fat_read(args, buf, 512) == 0) {
        kprintf("%s\n", buf);
    }
    
    free(buf);
}

void cmd_write(char *args) {
    char *content = split_args(args);
    if (!content) {
        kprintf("Usage: write <filename> <text>\n");
        return;
    }
    
    // Simple quote stripping if present
    if (content[0] == '"') {
        content++;
        int len = strlen(content);
        if (len > 0 && content[len-1] == '"') content[len-1] = 0;
    }
    
    if (fat_write(args, (uint8_t*)content, strlen(content)) == 0) {
        kprintf("Written to %s\n", args);
    } else {
        kprintf("Write failed.\n");
    }
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
    else if (strcmp(input_buffer, "ls") == 0) {
        cmd_ls();
    }
    else if (strncmp(input_buffer, "echo ", 5) == 0) {
        cmd_echo(input_buffer + 5);
    }
    else if (strcmp(input_buffer, "echo") == 0) {
        kprintf("\n");
    }
    else if (strncmp(input_buffer, "touch ", 6) == 0) {
        cmd_touch(input_buffer + 6);
    }
    else if (strncmp(input_buffer, "rm ", 3) == 0) {
        cmd_rm(input_buffer + 3);
    }
    else if (strncmp(input_buffer, "mv ", 3) == 0) {
        cmd_mv(input_buffer + 3);
    }
    else if (strncmp(input_buffer, "cp ", 3) == 0) {
        cmd_cp(input_buffer + 3);
    }
    else if (strncmp(input_buffer, "cat ", 4) == 0) {
        cmd_cat(input_buffer + 4);
    }
    else if (strncmp(input_buffer, "write ", 6) == 0) {
        cmd_write(input_buffer + 6);
    }
    else if (strncmp(input_buffer, "mem_dump ", 6) == 0) {
        cmd_memdump();
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