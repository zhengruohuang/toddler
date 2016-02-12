#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "common/include/data.h"
#include "common/include/floppyimg.h"


#define FLOPPY_SIZE 1474560
#define BUFFER_SIZE 512


static u8 *floppy;
static u8 *buffer;


static void copy_to_floppy(const u8 *p, u32 offset, u32 size)
{
    u32 i;
    
    for (i = 0; i < size; i++) {
        floppy[offset + i] = p[i];
    }
}

static void extract_file_name(u8 *extracted_name, u8* full_name)
{
    int i;
    
    int name_start = 0;
    int name_end = strlen(full_name);
    
    for (i = name_end; i >= 0; i--) {
        if (full_name[i] == '\\' || full_name[i] == '/') {
            name_start = i + 1;
            break;
        }
    }
    
    int name_length = name_end - name_start;
    if (name_length > 12) {
        name_length = 12;
    }
    
    for (i = 0; i < 12; i++) {
        extracted_name[i] = '\0';
    }
    
    for (i = 0; i < name_length; i++) {
        extracted_name[i] = full_name[name_start + i];
    }
}

static int generate_floppy(int argc, char *argv[])
{
    /* Initialize the image */
    floppy = (u8 *)malloc(FLOPPY_SIZE);
    buffer = (u8 *)malloc(BUFFER_SIZE);
    
    char *save_to = argv[1];
    
    FILE *file;
    u32 file_size;
    u32 count;
    u8 file_name[12];
    
    u32 current_offset = 0;
    u32 current_sector = 0;
    u8 *current_file;
    
    u32 i, j;
    
    /* Calculate FAT count and file count */
    u32 file_count = argc - 3;
    u32 fat_count = 1;
    if (file_count > 31) {
        u32 file_count_for_slave_fat = file_count - 31;
        fat_count += file_count_for_slave_fat / 32;
        
        if (file_count_for_slave_fat % 32) {
            fat_count++;
        }
    }
    printf("\tFile Count: %d\n", file_count);
    printf("\tFAT Count: %d\n", fat_count);
    
    /* Boot File */
    printf("\tProcessing Boot File: %s ... ", argv[2]);
    
    file_size = 0;
    file = fopen(argv[2], "rb");
    if (0 == file) {
        printf("Failed!\n");
        return 1;
    }
    fseek(file, 0, 0);
    
    do {
        /* Read the file */
        count = fread(buffer, 1, BUFFER_SIZE, file);
        
        /* Copy the file to floppy image */
        copy_to_floppy(buffer, current_offset, count);
        
        /* Set size and position */
        current_offset += count;
        file_size += count;
    } while (!feof(file));
    
    fclose(file);
    printf("Done!\n");
    
    /* Process FATs */
    current_sector = 16;
    current_offset = current_sector * 512;
    struct floppy_fat_master *fat_master = (struct floppy_fat_master *)((unsigned long)floppy + current_offset);
    fat_master->header.fat_count = fat_count;
    fat_master->header.file_count = file_count;
    
    /* Normal Files */
    current_sector = 16 + fat_count;
    current_offset = current_sector * 512;
    
    for (i = 0; i < file_count; i++) {
        current_file = argv[3 + i];
        printf("\tProcessing File: %s ... ", current_file);
        
        if (0 == current_file) {
            printf("Failed!\n");
            return -1;
        }
        
        /* Load the file */
        file_size = 0;
        file = fopen(current_file, "rb");
        if (0 == file) {
            printf("Failed!\n");
            return -1;
        }
        
        do {
            /* Read the file */
            count = fread(buffer, 1, BUFFER_SIZE, file);
            
            /* Copy the file to floppy image */
            copy_to_floppy(buffer, current_offset, count);
            
            /* Set size and position */
            current_offset += count;
            file_size += count;
        } while (!feof(file));
        
        fclose(file);
        
        /* Calculate file info */
        u32 sector_count = file_size / 512;
        if (file_size % 512) {
            sector_count++;
        }
        
        /* Extract file name */
        file_name[12];
        extract_file_name(file_name, current_file);
        
        /* Set file info in FAT */
        if (i < 31) {
            fat_master->entries[i].start_sector = current_sector;
            fat_master->entries[i].sector_count = sector_count;
            
            for (j = 0; j < 12; j++) {
                fat_master->entries[i].file_name[j] = file_name[j];
            }
        } else {
            u32 slave_index = (i - 31) / 32;
            u32 slave_i = i - 31 - 32 * slave_index;
            struct floppy_fat_slave *fat_slave = (struct floppy_fat_slave *)((unsigned long)fat_master + 512 * slave_index);
            
            fat_slave->entries[slave_i].start_sector = current_sector;
            fat_slave->entries[slave_i].sector_count = sector_count;
            
            for (j = 0; j < 12; j++) {
                fat_slave->entries[i].file_name[j] = file_name[j];
            }
        }
        
        /* Update our position */
        current_sector += sector_count;
        current_offset = current_offset = current_sector * 512;
        
        printf("Done!\n");
    }
    
    /* Save the floppy image */
    printf("\tWriting to image: %s ... ", argv[1]);
    
    file = fopen(argv[1], "wb");
    if (0 == file) {
        printf("Failed!\n");
        return 1;
    }
    
    fwrite(floppy, FLOPPY_SIZE, 1, file);
    
    fclose(file);
    printf("Done!\n");
    
    return 0;
}

static void print_usage()
{
    printf("Usage\n");
    printf("\tfloppyimg floppy_image boot_file [file1, file2, ..., file n]\n");
}

int main(int argc, char *argv[])
{
    printf("Toddler Floppy Image Generator 0.5.0.1\n");
    printf("Copyright 2016 Ruohuang Zheng\n");
    
    /* Check arguments */
    if (argc < 3) {
        print_usage();
    } else {
        int result = generate_floppy(argc, argv);
        
        if (result) {
            printf("\tUnable to generate floppy image!\n");
        } else {
            printf("\tFloppy image generated successfully!\n");
        }
    }
    
    return 0;
}
