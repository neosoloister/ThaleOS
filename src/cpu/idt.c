#include "idt.h"

static idt_entry_t idt[256];
static idt_ptr_t idt_ptr;

extern void idt_flush(uint32_t idt_ptr_addr);

void idt_set_gate(uint8_t vec, uint32_t handler, uint16_t selector, uint8_t flags) {
    idt[vec].offset_low  = (uint16_t)(handler & 0xFFFF);
    idt[vec].selector    = selector;
    idt[vec].zero        = 0;
    idt[vec].flags       = flags;
    idt[vec].offset_high = (uint16_t)((handler >> 16) & 0xFFFF);
}

void idt_init(void) {
    idt_ptr.limit = (uint16_t)(sizeof(idt) - 1);
    idt_ptr.base  = (uint32_t)&idt;

    // clear IDT
    for (int i = 0; i < 256; i++) {
        idt[i].offset_low  = 0;
        idt[i].selector    = 0;
        idt[i].zero        = 0;
        idt[i].flags       = 0;
        idt[i].offset_high = 0;
    }

    idt_flush((uint32_t)&idt_ptr);
}