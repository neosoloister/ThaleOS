#ifndef FAT_H
#define FAT_H

#include <stdint.h>

typedef struct {
    uint8_t jump_instruction[3];
    uint8_t oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t number_of_fats;
    uint16_t root_dir_entries;
    uint16_t total_sectors_small;
    uint8_t media_descriptor;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t number_of_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_large;
    
    uint8_t drive_number;
    uint8_t reserved;
    uint8_t signature;
    uint32_t volume_id;
    uint8_t volume_label[11];
    uint8_t fs_type[8];
} __attribute__((packed)) fat_bpb_t;

typedef struct {
    uint8_t filename[8];
    uint8_t ext[3];
    uint8_t attributes;
    uint8_t reserved;
    uint8_t creation_time_tenth;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access_date;
    uint16_t first_cluster_high;
    uint16_t last_sync_time;
    uint16_t last_sync_date;
    uint16_t first_cluster_low;
    uint32_t file_size;
} __attribute__((packed)) fat_entry_t;

void fat_init();
void fat_list_root();
int fat_create(char *filename);
int fat_delete(char *filename);
int fat_rename(char *old_name, char *new_name);
int fat_copy(char *src, char *dest);
int fat_exists(char *filename);
int fat_read(char *filename, uint8_t *buffer, uint32_t count);

#endif
