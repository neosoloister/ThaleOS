#include "rtc.h"
#include "../cpu/ports.h"

#define CMOS_ADDRESS 0x70
#define CMOS_DATA    0x71

static int get_update_in_progress_flag() {
    port_byte_out(CMOS_ADDRESS, 0x0A);
    return (port_byte_in(CMOS_DATA) & 0x80);
}

static uint8_t get_rtc_register(int reg) {
    port_byte_out(CMOS_ADDRESS, reg);
    return port_byte_in(CMOS_DATA);
}

void rtc_get_time(rtc_time_t *time) {
    uint8_t registerB;
    uint8_t second, minute, hour, day, month, year;
    uint8_t last_second, last_minute, last_hour, last_day, last_month, last_year;

    while (get_update_in_progress_flag());
    second = get_rtc_register(0x00);
    minute = get_rtc_register(0x02);
    hour = get_rtc_register(0x04);
    day = get_rtc_register(0x07);
    month = get_rtc_register(0x08);
    year = get_rtc_register(0x09);

    do {
        last_second = second;
        last_minute = minute;
        last_hour = hour;
        last_day = day;
        last_month = month;
        last_year = year;

        while (get_update_in_progress_flag());
        second = get_rtc_register(0x00);
        minute = get_rtc_register(0x02);
        hour = get_rtc_register(0x04);
        day = get_rtc_register(0x07);
        month = get_rtc_register(0x08);
        year = get_rtc_register(0x09);
    } while ((last_second != second) || (last_minute != minute) || (last_hour != hour) ||
             (last_day != day) || (last_month != month) || (last_year != year));

    registerB = get_rtc_register(0x0B);

    // Convert BCD to binary values if necessary
    if (!(registerB & 0x04)) {
        second = (second & 0x0F) + ((second / 16) * 10);
        minute = (minute & 0x0F) + ((minute / 16) * 10);
        hour = ((hour & 0x0F) + (((hour & 0x70) / 16) * 10)) | (hour & 0x80);
        day = (day & 0x0F) + ((day / 16) * 10);
        month = (month & 0x0F) + ((month / 16) * 10);
        year = (year & 0x0F) + ((year / 16) * 10);
    }

    // Convert 12 hour clock to 24 hour clock if necessary
    if (!(registerB & 0x02) && (hour & 0x80)) {
        hour = ((hour & 0x7F) + 12) % 24;
    }

    // Calculate full (4-digit) year
    // CMOS year is 2 digits. Assume 20XX for now.
    // In a real OS you typically read the century register but that varies.
    // ThaleOS is likely modern enough that we can assume 21st century or just report 2 digit.
    // Let's keep it simple and just store the 2 digit or simple conversion for now.
    // The struct has uint8_t year, so we fit 2 digits perfectly.
    
    time->second = second;
    time->minute = minute;
    time->hour = hour;
    time->day = day;
    time->month = month;
    time->year = year;
}
