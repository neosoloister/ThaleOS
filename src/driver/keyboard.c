#include "keyboard.h"
#include "../cpu/ports.h"
#include "../cpu/isr.h"
#include "../lib/kprintf.h"

static void keyboard_callback(registers_t *regs) {
    /* The PIC leaves us the scancode in port 0x60 */
    uint8_t scancode = port_byte_in(0x60);
    

    char sc_ascii[] = { '?', '?', '1', '2', '3', '4', '5', '6',     
        '7', '8', '9', '0', '-', '=', '?', '?', 'Q', 'W', 'E', 'R', 'T', 'Y', 
            'U', 'I', 'O', 'P', '[', ']', '?', '?', 'A', 'S', 'D', 'F', 'G', 
            'H', 'J', 'K', 'L', ';', '\'', '`', '?', '\\', 'Z', 'X', 'C', 'V', 
            'B', 'N', 'M', ',', '.', '/', '?', '?', '?', ' '};

    if (scancode > 57) return;

    // kprintf("Keyboard scancode: %x, ", scancode);
    char letter = sc_ascii[(int)scancode];
    char str[2] = {letter, '\0'};
    kprintf("%s", str);
    
    (void)regs;
}

void init_keyboard(void) {
   register_interrupt_handler(IRQ1, keyboard_callback); 
}
