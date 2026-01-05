#include "kprintf.h"
#include "../driver/vga.h"
#include <stdarg.h>
#include <stdbool.h>

static void itoa(int num) {
    char buff[12];
    bool sign = 0;
    int i = 0;
    if (num < 0) {
        sign = 1;
        num *= -1;
    }
    do {
        buff[i++] = (char)('0' + (num % 10));
        num /= 10;
    } while (num != 0);

    if (sign)
        buff[i++] = '-';
    for (int l = 0, r = i - 1; l < r; l++, r--) {
        char tmp = buff[l];
        buff[l] = buff[r];
        buff[r] = tmp;
    }
    buff[i] = '\0';
    vga_write(buff);
}

static void htoa(uint32_t hex) {
    static const char digits[] = "0123456789ABCDEF";
    char buff[9];

    for (int i = 7; i >= 0; --i) {
        buff[i] = digits[hex & 0xF];
        hex >>= 4;
    }
    buff[8] = '\0';
    vga_write(buff);
}


static void ftoa(double f, int decimals) {
    if (f < 0) {
        vga_putc('-');
        f *= -1;
    }

    int int_part = (int)f;
    itoa(int_part);
    
    if (decimals > 0) {
        vga_putc('.');
        f -= int_part;
        for (int i = 0; i < decimals; i++) {
            f *= 10;
            int digit = (int)f;
            vga_putc('0' + digit);
            f -= digit;
        }
    }
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
        switch (*fmt) {
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
        case 'x': {
            uint32_t x = va_arg(args, uint32_t);
            htoa(x);
            break;

        }
        case 'p': {
            void *p = va_arg(args, void *);
            vga_write("0x");
            htoa((uint32_t)p);
            break;
        }
        case 'f': {
            double f = va_arg(args, double);
            ftoa(f, 6);
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