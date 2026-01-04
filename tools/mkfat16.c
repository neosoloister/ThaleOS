#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    uint8_t jump[3];
    uint8_t oem[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fats;
    uint16_t root_entries;
    uint16_t total_sectors_small;
    uint8_t media;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t heads;
    uint32_t hidden;
    uint32_t total_sectors_large;
    uint8_t drive;
    uint8_t reserved;
    uint8_t signature;
    uint32_t vol_id;
    uint8_t label[11];
    uint8_t type[8];
} __attribute__((packed)) bpb_t;

typedef struct {
    uint8_t name[8];
    uint8_t ext[3];
    uint8_t attr;
    uint8_t reserved;
    uint8_t ctime_ms;
    uint16_t ctime;
    uint16_t cdate;
    uint16_t adate;
    uint16_t cluster_hi;
    uint16_t mtime;
    uint16_t mdate;
    uint16_t cluster_lo;
    uint32_t size;
} __attribute__((packed)) dir_entry_t;

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <output_image>\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }

    uint32_t total_sectors = 20480;
    
    bpb_t bpb;
    memset(&bpb, 0, sizeof(bpb));
    bpb.jump[0] = 0xEB; bpb.jump[1] = 0x3C; bpb.jump[2] = 0x90;
    memcpy(bpb.oem, "THALEOS ", 8);
    bpb.bytes_per_sector = 512;
    bpb.sectors_per_cluster = 1; 
    bpb.reserved_sectors = 1;    
    bpb.fats = 2;
    bpb.root_entries = 512;
    bpb.total_sectors_small = total_sectors;
    bpb.media = 0xF8; 
    bpb.sectors_per_fat = 80; 
    bpb.sectors_per_track = 32;
    bpb.heads = 64;
    bpb.hidden = 0;
    bpb.total_sectors_large = 0; 
    
    bpb.drive = 0x80;
    bpb.signature = 0x29;
    bpb.vol_id = 0x12345678;
    memcpy(bpb.label, "THALEOSDISK", 11);
    memcpy(bpb.type, "FAT16   ", 8);

    uint8_t boot_sector[512];
    memset(boot_sector, 0, 512);
    memcpy(boot_sector, &bpb, sizeof(bpb));
    boot_sector[510] = 0x55;
    boot_sector[511] = 0xAA;

    fwrite(boot_sector, 1, 512, f);

    uint16_t *fat = malloc(bpb.sectors_per_fat * 512);
    memset(fat, 0, bpb.sectors_per_fat * 512);
    fat[0] = 0xFFF8;
    fat[1] = 0xFFFF;

    fat[2] = 0xFFFF; 

    fwrite(fat, 1, bpb.sectors_per_fat * 512, f);
    fwrite(fat, 1, bpb.sectors_per_fat * 512, f);
    
    free(fat);

    dir_entry_t *root = malloc(bpb.root_entries * 32);
    memset(root, 0, bpb.root_entries * 32);

    memcpy(root[0].name, "TEST    ", 8);
    memcpy(root[0].ext, "TXT", 3);
    root[0].attr = 0x20; 
    root[0].cluster_lo = 2;
    root[0].size = 14; 
    
    fwrite(root, 1, bpb.root_entries * 32, f);
    free(root);

    uint8_t *data_cluster = malloc(512);
    memset(data_cluster, 0, 512);
    strcpy((char*)data_cluster, "Hello ThaleOS!");
    
    fwrite(data_cluster, 1, 512, f);
    free(data_cluster);

    uint32_t written_sectors = 1 + bpb.fats * bpb.sectors_per_fat + (bpb.root_entries * 32)/512 + 1;
    uint32_t remaining = total_sectors - written_sectors;
    
    uint8_t zero[512] = {0};
    for (uint32_t i = 0; i < remaining; i++) {
        fwrite(zero, 1, 512, f);
    }

    fclose(f);
    printf("Created FAT16 image: %s\n", argv[1]);
    return 0;
}
