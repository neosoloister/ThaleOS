#ifndef ATA_H
#define ATA_H

#include <stdint.h>

void ata_read_sector(int drive, uint32_t lba, uint8_t *buffer);
void ata_write_sector(int drive, uint32_t lba, uint8_t *buffer);

#endif
