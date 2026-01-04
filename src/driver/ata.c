#include "ata.h"
#include "../cpu/ports.h"

#define ATA_PRIMARY_DATA         0x1F0
#define ATA_PRIMARY_ERR          0x1F1
#define ATA_PRIMARY_SEC_COUNT    0x1F2
#define ATA_PRIMARY_LBA_LO       0x1F3
#define ATA_PRIMARY_LBA_MID      0x1F4
#define ATA_PRIMARY_LBA_HI       0x1F5
#define ATA_PRIMARY_DRIVE_HEAD   0x1F6
#define ATA_PRIMARY_STATUS       0x1F7
#define ATA_PRIMARY_COMMAND      0x1F7

#define ATA_CMD_READ_PIO         0x20
#define ATA_CMD_WRITE_PIO        0x30

static void ata_wait_bsy() {
    while (port_byte_in(ATA_PRIMARY_STATUS) & 0x80);
}

static void ata_wait_drq() {
    while (!(port_byte_in(ATA_PRIMARY_STATUS) & 0x08));
}

void ata_read_sector(int drive, uint32_t lba, uint8_t *buffer) {
    ata_wait_bsy();

    uint8_t drive_cmd = (drive == 0) ? 0xE0 : 0xF0;
    port_byte_out(ATA_PRIMARY_DRIVE_HEAD, drive_cmd | ((lba >> 24) & 0x0F));
    port_byte_out(ATA_PRIMARY_SEC_COUNT, 1);
    port_byte_out(ATA_PRIMARY_LBA_LO, (uint8_t) lba);
    port_byte_out(ATA_PRIMARY_LBA_MID, (uint8_t)(lba >> 8));
    port_byte_out(ATA_PRIMARY_LBA_HI, (uint8_t)(lba >> 16));
    port_byte_out(ATA_PRIMARY_COMMAND, ATA_CMD_READ_PIO);

    ata_wait_bsy();
    ata_wait_drq();

    for (int i = 0; i < 256; i++) {
        uint16_t data = port_word_in(ATA_PRIMARY_DATA);
        ((uint16_t *) buffer)[i] = data;
    }
}

void ata_write_sector(int drive, uint32_t lba, uint8_t *buffer) {
    ata_wait_bsy();

    uint8_t drive_cmd = (drive == 0) ? 0xE0 : 0xF0;
    port_byte_out(ATA_PRIMARY_DRIVE_HEAD, drive_cmd | ((lba >> 24) & 0x0F));
    port_byte_out(ATA_PRIMARY_SEC_COUNT, 1);
    port_byte_out(ATA_PRIMARY_LBA_LO, (uint8_t) lba);
    port_byte_out(ATA_PRIMARY_LBA_MID, (uint8_t)(lba >> 8));
    port_byte_out(ATA_PRIMARY_LBA_HI, (uint8_t)(lba >> 16));
    port_byte_out(ATA_PRIMARY_COMMAND, ATA_CMD_WRITE_PIO);

    ata_wait_bsy();
    
    for (int i = 0; i < 256; i++) {
        port_word_out(ATA_PRIMARY_DATA, ((uint16_t *) buffer)[i]);
        // Simple delay/flush might be needed on older hardware, usually fine here
        __asm__ volatile("nop; nop;");
    }
}
