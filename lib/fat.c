#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <linux/hdreg.h>

#include "blkDevices.h"

#define FAT12_16_BOOTCODE_SIZE  448
#define FAT32_BOOTCODE_SIZE     420
#define OEM_NAME "MSWIN4.1"
#define SECTOR_SIZE             512
#define SECTORS_PER_FAT         1 // FIXME: This is a placeholder
#define ROOT_CLUSTER            2
#define FS_INFO_OFFSET_32       1
#define BOOT_SIGNATURE          0xAA55
#define BOOTSECTOR_SIGNATURE    0x29
#define MEDIA_TYPE              0xF8

#define FAT32_FS_TYPE "FAT32   "
#define FAT16_FS_TYPE "FAT16   "
#define FAT12_FS_TYPE "FAT12   "

#define FSINFO_LEAD_SIGNATURE 0x41615252
#define FSINFO_STRUCT_SIGNATURE 0x61417272
#define FSINFO_TRAIL_SIGNATURE 0xAA550000
#define FSINFO_UNKNOWN 0xFFFFFFFF

const uint8_t FAT32_JMP_INSTRUCTION[3] = {0xEB, 0x58, 0x90};
const uint8_t FAT12_16_JMP_INSTRUCTION[3] = {0xEB, 0x3C, 0x90};

static ssize_t zero_disk(int* fd);
static ssize_t boot_sector(int* fd, uint8_t FATVersion, uint8_t clusterSectors, char volumeLabel[11]);
static ssize_t writeFsInfo(int* fd, uint16_t fsInfoSector);

// Binary Ganerated from src/boot.asm
uint8_t bootCode12_16[FAT12_16_BOOTCODE_SIZE] = {
    0xBE, 0x00, 0x00, 0xB4, 0x0E, 0x8A, 0x84, 0x19, 0x7C, 0xCD, 0x10, 0x83, 0xC6, 0x01, 0x80, 
    0xBC, 0x19, 0x7C, 0x00, 0x75, 0xEE, 0xCD, 0x18, 0xEB, 0xFE, 0x54, 0x68, 0x69, 0x73, 0x20, 
    0x69, 0x73, 0x20, 0x6E, 0x6F, 0x74, 0x20, 0x61, 0x20, 0x62, 0x6F, 0x6F, 0x74, 0x61, 0x62, 
    0x6C, 0x65, 0x20, 0x64, 0x69, 0x73, 0x6B, 0x0A, 0x0D
};

uint8_t bootCode32[FAT32_BOOTCODE_SIZE] = {
    0xBE, 0x00, 0x00, 0xB4, 0x0E, 0x8A, 0x84, 0x19, 0x7C, 0xCD, 0x10, 0x83, 0xC6, 0x01, 0x80, 
    0xBC, 0x19, 0x7C, 0x00, 0x75, 0xEE, 0xCD, 0x18, 0xEB, 0xFE, 0x54, 0x68, 0x69, 0x73, 0x20, 
    0x69, 0x73, 0x20, 0x6E, 0x6F, 0x74, 0x20, 0x61, 0x20, 0x62, 0x6F, 0x6F, 0x74, 0x61, 0x62, 
    0x6C, 0x65, 0x20, 0x64, 0x69, 0x73, 0x6B, 0x0A, 0x0D
};

struct fsInfo {
    uint32_t lead_signiture; /* 0x41615252, Confirms that this is a fsInfo sector */
    uint8_t reserved[480]; /* unused */
    uint32_t struct_signiture; /* 0x61417272, Another struct signiture */
    uint32_t free_clusters; /* Number of free clusters on the disk */
    uint32_t next_cluster; /* Next free cluster */
    uint8_t reserved2[12]; /* unused */
    uint32_t trail_signiture; /* 0xAA550000 */
} __attribute__((packed));

struct msdos_volume_info {
    uint8_t drive_number; /* BIOS drive number */
    uint8_t reserved; /* idk what this does, 80s microsoft voodoo */
    uint8_t boot_signiture; /* Again magic voodoo, should just be 0x29 */
    uint32_t volume_serial; /* Drive serial number, used for identification */
    char label[11]; /* Drive label, used for identificaton */
    char fs_type[8]; /* Generally not used on modern systems as far as I can tell */
}__attribute__((packed));

struct msdos_boot_sector {
    uint8_t jump_boot[3]; /* Jump instruction to boot code */
    char OEM_name[8]; /* OEM Name Identifier */
    uint16_t sector_bytes; /* Bytes per sector, 512, 1024, 2048 or 4096 */
    uint8_t cluster_sectors; /* Sectors per allocation unit */
    uint16_t reserved_sectors; /* Sectors in reserved area, 1 on FAT32/16, usually 16 on FAT32 */
    uint8_t FAT; /* Number of FATs, should be 2 */
    uint16_t root_entries; /* 32-byte directory entries in root directory, FAT12/16 ONLY */
    uint16_t total_sectors_16; /* Number of sectors (16-bit <0x10000) */ 
    uint8_t media; /* unused on modern systems */
    uint16_t FAT_size; /* Sectors used by FAT, FAT12/16 ONLY */
    uint16_t track_sectors; /* unused on modern systems */
    uint16_t heads; /* unused on modern systems */
    uint32_t hidden_sectors; /* Sectors preceding the FAT volume */
    uint32_t total_sectors; /* Total number of sectors (32-bit) */
    union {
        struct oldfat {
                struct msdos_volume_info vol_info;  
                uint8_t boot_code[FAT12_16_BOOTCODE_SIZE]; /* Bootstrap Program */
        } __attribute__((packed)) oldfat;
        struct fat32 {
            uint32_t FAT32_size; /* Size of FAT in sectors */
            uint16_t flags; /* Bit 7: FAT Mirroring, Bits 0-3: Active FAT */
            uint8_t FS_ver[2]; /* FAT32 Version */
            uint32_t root_cluster; /* First cluster in root dir */
            uint16_t FS_info; /* FSInfo struct sector, offset from boot sector */
            uint16_t backup_boot_sector; /* Sector of backup boot sector, offset from boot sector */
            uint8_t reserved[12]; /* unused */
            struct msdos_volume_info vol_info;
            uint8_t boot_code[FAT32_BOOTCODE_SIZE];
        } __attribute__((packed)) fat32;
    };
    uint16_t sign; /* Boot Signiture, 0xAA55 */
}__attribute__((packed));

int fat_format(char* target, int fatVersion, uint8_t clusterSectors, char volumeLabel[11], bool quick)
{

    int fd = open(target, O_WRONLY);
    if (fd == -1)
    {
        perror("open");
        return 1;
    }


    if(!quick)
    {
        zero_disk(&fd);
    }

    printf("clusterSize: %d\n", clusterSectors);
    boot_sector(&fd, fatVersion, clusterSectors, volumeLabel);

    if (fatVersion == 32)
    {
        writeFsInfo(&fd, FS_INFO_OFFSET_32);
    }

    return 0;
}

static ssize_t zero_disk(int* fd)
{
    char* zeros = calloc(1, SECTOR_SIZE);
    if (zeros == NULL)
    {
        perror("Memory Allocation Error");
        return -1;
    }

    ssize_t written, total = 0;
    do {
        total += written = write(*fd, zeros, 512); // Write 0s to each sector
    } while (written == 512);

    free(zeros);
    return 0;
}

static ssize_t writeFsInfo(int* fd, uint16_t fsInfoSector)
{
    struct fsInfo* fsInfo = malloc(sizeof(struct fsInfo));
    // zero out reserved
    memset(fsInfo->reserved, 0, 480);
    memset(fsInfo->reserved2, 0, 12);
    fsInfo->lead_signiture = FSINFO_LEAD_SIGNATURE;
    fsInfo->struct_signiture = FSINFO_STRUCT_SIGNATURE;
    fsInfo->free_clusters = FSINFO_UNKNOWN;
    fsInfo->next_cluster = FSINFO_UNKNOWN;
    fsInfo->trail_signiture = FSINFO_TRAIL_SIGNATURE;

    /* Write fsInfo to disk */
    lseek(*fd, fsInfoSector * SECTOR_SIZE, SEEK_SET);
    int status = write(*fd, fsInfo, sizeof(struct fsInfo));
    if (status == -1)
    {
        perror("write");
        return -1;
    }

    free(fsInfo);
    return 0;
}

static ssize_t boot_sector(
        int* fd, uint8_t FATVersion, uint8_t clusterSectors, char volumeLabel[11])
{

    // Get disk info
    unsigned long long blocks = get_dev_blocks(fd);
    struct hd_geometry geo = get_bios_params(fd);

    /* Set boot sector values */
    struct msdos_boot_sector* bootSector = malloc(sizeof(struct msdos_boot_sector));
    memcpy(bootSector->OEM_name, OEM_NAME, 8);
    bootSector->sector_bytes = SECTOR_SIZE;
    bootSector->cluster_sectors = clusterSectors;
    bootSector->FAT = 2;
    bootSector->media = MEDIA_TYPE;
    bootSector->hidden_sectors = geo.start;
    bootSector->track_sectors = geo.sectors;
    bootSector->heads = geo.heads;

    if (FATVersion == 32)
    {
        printf("FAT32\n");
        bootSector->reserved_sectors = 32;
        bootSector->total_sectors = blocks;

        bootSector->fat32.FAT32_size = SECTORS_PER_FAT; // FIXME: This is a placeholder
        // bootSector->fat32.flags = 0
        bootSector->fat32.FS_ver[0] = 0;
        bootSector->fat32.FS_ver[1] = 0;
        bootSector->fat32.root_cluster = ROOT_CLUSTER;
        bootSector->fat32.FS_info = FS_INFO_OFFSET_32;
        bootSector->fat32.backup_boot_sector = 6;
        bootSector->fat32.vol_info.drive_number = 0x80;
        bootSector->fat32.vol_info.reserved = 0;
        bootSector->fat32.vol_info.boot_signiture = BOOTSECTOR_SIGNATURE;

        // Generate Volume Serial
        srand(time(NULL));
        bootSector->fat32.vol_info.volume_serial = rand();

        memcpy(bootSector->fat32.vol_info.label, volumeLabel, 11);
        memcpy(bootSector->fat32.vol_info.fs_type, FAT32_FS_TYPE, 8);

        memcpy(bootSector->jump_boot, FAT32_JMP_INSTRUCTION, 3);

        memcpy(bootSector->fat32.boot_code, bootCode32, FAT32_BOOTCODE_SIZE);
    }
    else
    {
        bootSector->reserved_sectors = 1;
        // Verify that the number of sectors is less than 0x10000
        /*
        if (geo.sectors > 0x10000)
        {
            fprintf(stderr, "Number of sectors exceeds 0x10000\n");
            return -1;
        }
        */
        bootSector->total_sectors_16 = blocks;

        bootSector->oldfat.vol_info.drive_number = 0x80;
        bootSector->oldfat.vol_info.reserved = 0;
        bootSector->oldfat.vol_info.boot_signiture = BOOTSECTOR_SIGNATURE;

        // Generate Volume Serial
        srand(time(NULL));
        bootSector->oldfat.vol_info.volume_serial = rand();

        memcpy(bootSector->oldfat.vol_info.label, volumeLabel, 11);
        memcpy(bootSector->oldfat.vol_info.fs_type, FAT16_FS_TYPE, 8);

        memcpy(bootSector->jump_boot, FAT12_16_JMP_INSTRUCTION, 3);

        memcpy(bootSector->oldfat.boot_code, bootCode12_16, FAT12_16_BOOTCODE_SIZE);
    }
    bootSector->sign = BOOT_SIGNATURE;

    /* Write boot sector to disk */
    lseek(*fd, 0, SEEK_SET); // Seek to beginning of disk
    int status = write(*fd, bootSector, SECTOR_SIZE);
    if (status == -1)
    {
        perror("write");
        return -1;
    }

    free(bootSector);
    return 0;
}
