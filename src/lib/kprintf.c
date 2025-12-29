#include "kprintf.h"
#include "../driver/vga.h"
#include <stdarg.h>
#include <stdbool.h>

void itoa (int num) {
    char buff[12];
    bool sign = 0;
    int i = 0;
    if (num < 0){
        sign = 1;
        num *= -1;
    }
    do {
        buff[i++] = (char)('0' + (num % 10));
        num /= 10;
    } while (num != 0);

    if (sign) buff[i++] = '-';
    for (int l = 0, r = i - 1; l < r; l++, r--) {
        char tmp = buff[l];
        buff[l] = buff[r];
        buff[r] = tmp;
    }
    buff[i] = '\0';
    vga_write(buff);
}

void kprintf(char *fmt, ...) {
    va_list args;

    va_start(args, fmt);
    while (*fmt) {
        if (*fmt != '%') {
            vga_putc(*fmt);
            fmt++;
            continue;
        }
        fmt++;
        switch (*fmt)
        {
        case 'd': {
            int d = va_arg(args, int);
            itoa(d);
            break;
        }
        case 'c': {
            char c = va_arg(args, int);
            vga_putc(c);
            break;
        }
        case 's': {
            char *s = va_arg(args, char *);
            vga_write(s ? s : "(null)");
            break;
        }
        case '%': {
            vga_putc('%');
            break;
        }
        default:
            break;
        }
        fmt++;
    }
    va_end(args);
}