#ifndef BLKDEVICES_H
#define BLKDEVICES_H

#include <sys/types.h>

ssize_t get_dev_blocks(int* fd);

#endif

