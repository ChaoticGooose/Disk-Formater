#ifndef FAT_H
#define FAT_H

int fat_format(char* target, int fatVersion, int clusterSize, int procSectors, char volumeLabel[11], bool quick);

#endif
