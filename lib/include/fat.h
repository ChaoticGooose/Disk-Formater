#ifndef FAT_H
#define FAT_H

#include <stdbool.h>
#include <stdint.h>

int fat_format(char* target, int fatVersion, uint8_t clusterSectors, char volumeLabel[11], bool quick);

#endif
