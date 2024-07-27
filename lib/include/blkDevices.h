#ifndef BLKDEVICES_H
#define BLKDEVICES_H

#include <sys/types.h>

unsigned long long get_dev_blocks(int* fd);
struct hd_geometry get_bios_params(int* fd);

#endif

