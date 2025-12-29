#ifndef VGA_H
#define VGA_H
#include <stdint.h>

#define VGA_ROW     25
#define VGA_COLUMN  80

enum VGA_COLOR {
    VGA_BLACK           = 0,
    VGA_BLUE            = 1,
    VGA_GREEN           = 2,
    VGA_CYAN            = 3,
    VGA_RED             = 4,
    VGA_MAGENTA         = 5,
    VGA_BROWN           = 6,
    VGA_LIGHT_GREY      = 7,
    VGA_DARK_GREY       = 8,
    VGA_LIGHT_BLUE      = 9,
    VGA_LIGHT_GREEN     = 10,
    VGA_LIGHT_CYAN      = 11,
    VGA_LIGHT_RED       = 12,
    VGA_LIGHT_MAGENTA   = 13,
    VGA_YELLOW          = 14,
    VGA_WHITE           = 15,
};

void vga_set_attr(uint8_t fg, uint8_t bg);
void vga_putc(char c);
void vga_clear();
void vga_write(char *s);
void vga_scroll();

#endif