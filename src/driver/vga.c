#include "vga.h"

static volatile uint16_t *const VGA = (uint16_t *)0xB8000;
static uint8_t VGA_ATTR = 0x0F;

uint8_t cursor_row = 0;
uint8_t cursor_column = 0;

static inline uint8_t vga_attr(uint8_t fg, uint8_t bg) {
  return (bg << 4) | (fg & 0x0F);
}

static inline uint16_t vga_entry(char c, uint8_t attr) {
  return (uint16_t)c | (uint16_t)attr << 8;
}

static inline int vga_index(uint8_t row, uint8_t col) {
  return row * VGA_COLUMN + col;
}

void vga_set_attr(uint8_t fg, uint8_t bg) { VGA_ATTR = vga_attr(fg, bg); }

void vga_set_cursor(uint8_t row, uint8_t col) {
  if (row >= VGA_ROW)
    row = VGA_ROW - 1;
  if (col >= VGA_COLUMN)
    col = VGA_COLUMN - 1;
  cursor_row = row;
  cursor_column = col;
}

void vga_putc(char c) {
  if (c == '\n') {
    cursor_column = 0;
    cursor_row++;

    if (cursor_row >= VGA_ROW) {
      vga_scroll();
      cursor_row = VGA_ROW - 1;
    }
    return;
  }
  if (cursor_column >= VGA_COLUMN) {
    cursor_column = 0;
    cursor_row++;

    if (cursor_row >= VGA_ROW) {
      vga_scroll();
      cursor_row = VGA_ROW - 1;
    }
  }
  int index = vga_index(cursor_row, cursor_column);
  VGA[index] = vga_entry(c, VGA_ATTR);
  cursor_column++;
}

void vga_write(char *s) {
  while (*s) {
    vga_putc(*s);
    s++;
  }
}

void vga_clear() {
  for (int r = 0; r < VGA_ROW; r++) {
    for (int c = 0; c < VGA_COLUMN; c++) {
      int index = vga_index(r, c);
      VGA[index] = vga_entry(' ', 0x0F);
    }
  }
}

void vga_scroll() {
  for (int r = 1; r < VGA_ROW; r++) {
    for (int c = 0; c < VGA_COLUMN; c++) {
      VGA[vga_index(r - 1, c)] = VGA[vga_index(r, c)];
    }
  }
  for (int c = 0; c < VGA_COLUMN; c++) {
    VGA[vga_index(VGA_ROW - 1, c)] = vga_entry(' ', VGA_ATTR);
  }
}

void vga_fill(uint8_t fg, uint8_t bg) {
  for (int r = 0; r < VGA_ROW; r++) {
    for (int c = 0; c < VGA_COLUMN; c++) {
      int index = vga_index(r, c);
      VGA[index] = vga_entry(' ', vga_attr(fg, bg));
    }
  }
}