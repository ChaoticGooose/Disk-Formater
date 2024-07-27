#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <stdio.h>
#include <linux/hdreg.h>


/* Get number of sectors on fd
 * https://man7.org/linux/man-pages/man4/sd.4.html/
*/
ssize_t get_dev_blocks(int* fd)
{
    unsigned long long blocks = 0;
    if (ioctl(*fd, BLKGETSIZE, &blocks) == -1)
    {
        perror("ioctl");
        return -1;
    }
    return blocks;
}

struct hd_geometry get_bios_params(int* fd)
{
    struct hd_geometry geo;
    if (ioctl(*fd, HDIO_GETGEO, &geo) == -1)
    {
        perror("ioctl");
    }
    return geo;
}

