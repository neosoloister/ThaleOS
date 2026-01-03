#include "keyboard.h"
#include "../cpu/isr.h"
#include "../cpu/ports.h"

static int shift_pressed = 0;

#define BUFFER_SIZE 256
static char key_buffer[BUFFER_SIZE];
static int buffer_head = 0;
static int buffer_tail = 0;

static void keyboard_callback(registers_t *regs) {
    uint8_t scancode = port_byte_in(0x60);

    // Shift
    if (scancode == 0x2A || scancode == 0x36) {
        shift_pressed = 1;
        return;
    } else if (scancode == 0xAA || scancode == 0xB6) {
        shift_pressed = 0;
        return;
    }

    // Ignore key releases
    if (scancode & 0x80)
        return;

    // Backspace, Enter
    char char_to_buffer = 0;

    if (scancode == 0x0E) {
        char_to_buffer = '\b';
    } else if (scancode == 0x1C) {
        char_to_buffer = '\n';
    } else {
        static char sc_ascii[] = {
            '?', '?', '1', '2', '3',  '4', '5', '6',  '7', '8', '9', '0',
            '-', '=', '?', '?', 'q',  'w', 'e', 'r',  't', 'y', 'u', 'i',
            'o', 'p', '[', ']', '?',  '?', 'a', 's',  'd', 'f', 'g', 'h',
            'j', 'k', 'l', ';', '\'', '`', '?', '\\', 'z', 'x', 'c', 'v',
            'b', 'n', 'm', ',', '.',  '/', '?', '?',  '?', ' '
        };

        static char sc_ascii_shift[] = {
            '?', '?', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
            '_', '+', '?', '?', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',
            'O', 'P', '{', '}', '?', '?', 'A', 'S', 'D', 'F', 'G', 'H',
            'J', 'K', 'L', ':', '"', '~', '?', '|', 'Z', 'X', 'C', 'V',
            'B', 'N', 'M', '<', '>', '?', '?', '?', '?', ' '
        };

        if (scancode <= 57) {
            char letter = shift_pressed ? sc_ascii_shift[(int)scancode]
                          : sc_ascii[(int)scancode];
            if (letter != '?') {
                char_to_buffer = letter;
            }
        }
    }

    if (char_to_buffer != 0) {
        // Buffer
        int next_head = (buffer_head + 1) % BUFFER_SIZE;
        if (next_head != buffer_tail) {
            key_buffer[buffer_head] = char_to_buffer;
            buffer_head = next_head;
        }
    }

    (void)regs;
}

char keyboard_getc(void) {
    if (buffer_head == buffer_tail) {
        return 0; // Buffer empty
    }

    char c = key_buffer[buffer_tail];
    buffer_tail = (buffer_tail + 1) % BUFFER_SIZE;
    return c;
}

void init_keyboard(void) {
    register_interrupt_handler(IRQ1, keyboard_callback);
}
