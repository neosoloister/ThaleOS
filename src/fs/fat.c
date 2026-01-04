#include "fat.h"
#include "../driver/ata.h"
#include "../lib/kprintf.h"
#include "../lib/mem.h"
#include "../lib/string.h"

#define FAT_DRIVE 1

static fat_bpb_t bpb;
static uint32_t root_dir_start_lba;
static uint32_t data_start_lba;
static uint32_t fat_start_lba;

static void to_dos_filename(const char *src, char *dest, char *ext_dest) {
    memset(dest, ' ', 8);
    memset(ext_dest, ' ', 3);
    
    int i = 0;
    int j = 0;
    while (src[i] && src[i] != '.' && j < 8) {
        char c = src[i];
        if (c >= 'a' && c <= 'z') c -= 32;
        dest[j++] = c;
        i++;
    }
    
    while (src[i] && src[i] != '.') i++;
    if (src[i] == '.') i++;
    
    j = 0;
    while (src[i] && j < 3) {
        char c = src[i];
        if (c >= 'a' && c <= 'z') c -= 32;
        ext_dest[j++] = c;
        i++;
    }
}

static void from_dos_filename(uint8_t *name, uint8_t *ext, char *dest) {
    int i;
    int j = 0;
    for (i = 0; i < 8; i++) {
        if (name[i] != ' ') dest[j++] = name[i];
    }
    dest[j++] = '.';
    for (i = 0; i < 3; i++) {
        if (ext[i] != ' ') dest[j++] = ext[i];
    }
    if (dest[j-1] == '.') dest[j-1] = 0;
    else dest[j] = 0;
}

void fat_init() {
    uint8_t *sector = malloc(512);
    ata_read_sector(FAT_DRIVE, 0, sector);

    memcpy(&bpb, sector, sizeof(fat_bpb_t));

    kprintf("FAT16 Filesystem detected on Drive %d\n", FAT_DRIVE);
    kprintf("  OEM: %.8s\n", bpb.oem_name);
    
    uint32_t fat_size = bpb.number_of_fats * bpb.sectors_per_fat;
    fat_start_lba = bpb.reserved_sectors;
    root_dir_start_lba = fat_start_lba + fat_size;
    
    uint32_t root_dir_size_sectors = (bpb.root_dir_entries * 32 + 511) / 512;
    data_start_lba = root_dir_start_lba + root_dir_size_sectors;

    free(sector);
}

static int fat_find_entry(char *filename, fat_entry_t *entry_out, uint32_t *lba_out, uint32_t *offset_out) {
    char dos_name[8];
    char dos_ext[3];
    to_dos_filename(filename, dos_name, dos_ext);

    uint32_t sectors_per_root = (bpb.root_dir_entries * 32 + 511) / 512;
    uint8_t *buffer = malloc(512);

    for (uint32_t i = 0; i < sectors_per_root; i++) {
        uint32_t lba = root_dir_start_lba + i;
        ata_read_sector(FAT_DRIVE, lba, buffer);
        
        fat_entry_t *entries = (fat_entry_t*)buffer;
        for (int j = 0; j < 16; j++) {
            if (entries[j].filename[0] == 0x00) break;
            if (entries[j].filename[0] == 0xE5) continue;
            if (entries[j].attributes == 0x0F) continue;

            if (memcmp(entries[j].filename, dos_name, 8) == 0 &&
                memcmp(entries[j].ext, dos_ext, 3) == 0) {
                
                if (entry_out) memcpy(entry_out, &entries[j], sizeof(fat_entry_t));
                if (lba_out) *lba_out = lba;
                if (offset_out) *offset_out = j * sizeof(fat_entry_t);
                
                free(buffer);
                return 0;
            }
        }
    }
    
    free(buffer);
    return 1;
}

int fat_exists(char *filename) {
    return fat_find_entry(filename, NULL, NULL, NULL) == 0;
}

void fat_list_root() {
    uint32_t sectors_per_root = (bpb.root_dir_entries * 32 + 511) / 512;
    uint8_t *buffer = malloc(512);
    
    kprintf("Listing Root Directory:\n");
    for (uint32_t i = 0; i < sectors_per_root; i++) {
        ata_read_sector(FAT_DRIVE, root_dir_start_lba + i, buffer);
        fat_entry_t *entries = (fat_entry_t*)buffer;
        
        for (int j = 0; j < 16; j++) {
            if (entries[j].filename[0] == 0x00) {
                 free(buffer);
                 return;
            }
            if (entries[j].filename[0] == 0xE5) continue;
            if (entries[j].attributes == 0x0F) continue;

            char name[13];
            from_dos_filename(entries[j].filename, entries[j].ext, name);
            
            if (entries[j].attributes & 0x10) {
                kprintf("%s <DIR>\n", name);
            } else {
                kprintf("%s %d\n", name, entries[j].file_size);
            }
        }
    }
    free(buffer);
}

static void fat_write_fat_entry(uint16_t cluster, uint16_t value) {
    uint32_t fat_offset = cluster * 2;
    uint32_t fat_sector = fat_start_lba + (fat_offset / 512);
    uint32_t ent_offset = fat_offset % 512;
    
    uint8_t *buffer = malloc(512);
    ata_read_sector(FAT_DRIVE, fat_sector, buffer);
    
    *(uint16_t*)(buffer + ent_offset) = value;
    
    ata_write_sector(FAT_DRIVE, fat_sector, buffer);
    ata_write_sector(FAT_DRIVE, fat_sector + bpb.sectors_per_fat, buffer);
    
    free(buffer);
}

static uint16_t fat_read_fat_entry(uint16_t cluster) {
    uint32_t fat_offset = cluster * 2;
    uint32_t fat_sector = fat_start_lba + (fat_offset / 512);
    uint32_t ent_offset = fat_offset % 512;
    
    uint8_t *buffer = malloc(512);
    ata_read_sector(FAT_DRIVE, fat_sector, buffer);
    
    uint16_t table_value = *(uint16_t*)(buffer + ent_offset);
    
    free(buffer);
    return table_value;
}

static uint16_t fat_find_free_cluster() {
    uint8_t *buffer = malloc(512);
    
    for (uint32_t i = 0; i < bpb.sectors_per_fat; i++) {
        ata_read_sector(FAT_DRIVE, fat_start_lba + i, buffer);
        uint16_t *fat_entries = (uint16_t*)buffer;
        
        for (int j = 0; j < 256; j++) {
            if (i == 0 && j < 2) continue;
            
            if (fat_entries[j] == 0x0000) {
                uint16_t cluster = i * 256 + j;
                free(buffer);
                return cluster;
            }
        }
    }
    
    free(buffer);
    return 0;
}

int fat_create(char *filename) {
    if (fat_exists(filename)) {
        kprintf("File already exists.\n");
        return 1;
    }

    char dos_name[8];
    char dos_ext[3];
    to_dos_filename(filename, dos_name, dos_ext);

    uint32_t sectors_per_root = (bpb.root_dir_entries * 32 + 511) / 512;
    uint8_t *buffer = malloc(512);

    for (uint32_t i = 0; i < sectors_per_root; i++) {
        uint32_t lba = root_dir_start_lba + i;
        ata_read_sector(FAT_DRIVE, lba, buffer);
        fat_entry_t *entries = (fat_entry_t*)buffer;

        for (int j = 0; j < 16; j++) {
            if (entries[j].filename[0] == 0x00 || entries[j].filename[0] == 0xE5) {
                memset(&entries[j], 0, sizeof(fat_entry_t));
                memcpy(entries[j].filename, dos_name, 8);
                memcpy(entries[j].ext, dos_ext, 3);
                entries[j].attributes = 0x20;
                
                ata_write_sector(FAT_DRIVE, lba, buffer);
                free(buffer);
                return 0;
            }
        }
    }
    
    free(buffer);
    kprintf("Root directory full!\n");
    return 1;
}

int fat_rename(char *old_name, char *new_name) {
    uint32_t lba, offset;
    fat_entry_t entry;
    
    if (fat_find_entry(old_name, &entry, &lba, &offset) != 0) {
        kprintf("File not found: %s\n", old_name);
        return 1;
    }
    
    if (fat_exists(new_name)) {
        kprintf("Destination exists: %s\n", new_name);
        return 1;
    }

    char dos_name[8];
    char dos_ext[3];
    to_dos_filename(new_name, dos_name, dos_ext);

    uint8_t *buffer = malloc(512);
    ata_read_sector(FAT_DRIVE, lba, buffer);
    fat_entry_t *target = (fat_entry_t*)(buffer + offset);
    
    memcpy(target->filename, dos_name, 8);
    memcpy(target->ext, dos_ext, 3);
    
    ata_write_sector(FAT_DRIVE, lba, buffer);
    free(buffer);
    return 0;
}

int fat_delete(char *filename) {
    uint32_t lba, offset;
    fat_entry_t entry;
    
    if (fat_find_entry(filename, &entry, &lba, &offset) != 0) {
        kprintf("File not found: %s\n", filename);
        return 1;
    }
    
    uint8_t *buffer = malloc(512);
    ata_read_sector(FAT_DRIVE, lba, buffer);
    fat_entry_t *target = (fat_entry_t*)(buffer + offset);
    
    target->filename[0] = 0xE5;
    
    ata_write_sector(FAT_DRIVE, lba, buffer);
    free(buffer);
    return 0;
}

int fat_copy(char *src, char *dest) {
    fat_entry_t entry;
    if (fat_find_entry(src, &entry, NULL, NULL) != 0) {
        kprintf("Source not found.\n");
        return 1;
    }
    
    if (entry.file_size == 0) {
        return fat_create(dest);
    }
    
    uint8_t *buffer = malloc(entry.file_size);
    if (!buffer) {
        kprintf("Memory allocation failed for copy.\n");
        return 1;
    }
    
    if (fat_read(src, buffer, entry.file_size) != 0) {
        kprintf("Failed to read source file.\n");
        free(buffer);
        return 1;
    }
    
    if (fat_write(dest, buffer, entry.file_size) != 0) {
        kprintf("Failed to write dest file.\n");
        free(buffer);
        return 1;
    }
    
    free(buffer);
    return 0;
}

int fat_read(char *filename, uint8_t *buffer, uint32_t count) {
    fat_entry_t entry;
    if (fat_find_entry(filename, &entry, NULL, NULL) != 0) {
        kprintf("File not found: %s\n", filename);
        return 1;
    }

    if (entry.file_size == 0) {
        return 0;
    }
    
    uint32_t read_size = count;
    if (read_size > entry.file_size) read_size = entry.file_size;
    
    uint16_t current_cluster = entry.first_cluster_low;
    uint32_t bytes_read = 0;
    
    while (bytes_read < read_size && current_cluster != 0xFFFF && current_cluster != 0) {
        uint32_t cluster_lba = data_start_lba + (current_cluster - 2) * bpb.sectors_per_cluster;

        
        // However, buffer is contiguous.
        // We read full cluster(s) into buffer
        
        // Actually, let's limit to 512 for sector read simplicity unless we loop sectors within cluster
        // fat_write assumes 512 bytes per cluster effectively (or writes only 1 sector).
        // Let's stick to 1 sector read per cluster iteration for safety with current framework, assuming 1 sector/cluster.
        
        uint8_t *sector_buf = malloc(512);
        ata_read_sector(FAT_DRIVE, cluster_lba, sector_buf); // Only reads 1 sector!
        
        uint32_t copy_size = 512;
        if (read_size - bytes_read < 512) copy_size = read_size - bytes_read;
        
        memcpy(buffer + bytes_read, sector_buf, copy_size);
        free(sector_buf);
        
        bytes_read += copy_size;
        
        // Find next cluster
        current_cluster = fat_read_fat_entry(current_cluster);
        if (current_cluster >= 0xFFF8) break; // End of chain
    }

    // Null terminate if possible and if space
    if (bytes_read < count) buffer[bytes_read] = 0;

    return 0;
}

int fat_write(char *filename, uint8_t *buffer, uint32_t count) {
    fat_entry_t entry;
    if (fat_find_entry(filename, &entry, NULL, NULL) != 0) {
        if (fat_create(filename) != 0) return 1;
        if (fat_find_entry(filename, &entry, NULL, NULL) != 0) return 1;
    }
    
    uint16_t current_cluster = entry.first_cluster_low;
    
    if (current_cluster == 0) {
        current_cluster = fat_find_free_cluster();
        if (current_cluster == 0) {
            kprintf("Disk full!\n");
            return 1;
        }
        
        fat_write_fat_entry(current_cluster, 0xFFFF);
        
        uint32_t dir_lba, dir_offset;
        fat_find_entry(filename, NULL, &dir_lba, &dir_offset);
        
        uint8_t *dir_buf = malloc(512);
        ata_read_sector(FAT_DRIVE, dir_lba, dir_buf);
        fat_entry_t *dir_ent = (fat_entry_t*)(dir_buf + dir_offset);
        dir_ent->first_cluster_low = current_cluster;
        dir_ent->file_size = 0;
        ata_write_sector(FAT_DRIVE, dir_lba, dir_buf);
        free(dir_buf);
    }
    
    uint32_t bytes_written = 0;
    
    while (bytes_written < count) {
        uint32_t cluster_lba = data_start_lba + (current_cluster - 2) * bpb.sectors_per_cluster;
        uint32_t chunk_size = 512;
        if (count - bytes_written < 512) chunk_size = count - bytes_written;
        
        uint8_t *sector_buf = malloc(512);
        if (chunk_size < 512) {
             ata_read_sector(FAT_DRIVE, cluster_lba, sector_buf);
        }
        memcpy(sector_buf, buffer + bytes_written, chunk_size);
        ata_write_sector(FAT_DRIVE, cluster_lba, sector_buf);
        free(sector_buf);
        
        bytes_written += chunk_size;
        
        if (bytes_written < count) {
            
            uint16_t new_cluster = fat_find_free_cluster();
            if (new_cluster == 0) {
                kprintf("Disk full during write!\n");
                break;
            }
            
            fat_write_fat_entry(current_cluster, new_cluster);
            fat_write_fat_entry(new_cluster, 0xFFFF);
            
            current_cluster = new_cluster;
        }
    }
    
    uint32_t dir_lba, dir_offset;
    fat_find_entry(filename, NULL, &dir_lba, &dir_offset);
    uint8_t *dir_buf = malloc(512);
    ata_read_sector(FAT_DRIVE, dir_lba, dir_buf);
    fat_entry_t *dir_ent = (fat_entry_t*)(dir_buf + dir_offset);
    dir_ent->file_size = bytes_written;
    ata_write_sector(FAT_DRIVE, dir_lba, dir_buf);
    free(dir_buf);
    
    return 0;
}
