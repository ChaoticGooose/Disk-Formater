#include <stdio.h>
#include <stdbool.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <sys/ioctl.h>

static int get_dev_blocks(int* fd);

int fat_format(char* target, bool quick)
{
    int fd = open(target, O_WRONLY);
    if (fd == -1)
    {
        perror("open");
        return 1;
    }

    int blocks = get_dev_blocks(&fd);
    printf("blocks: %d\n", blocks);

    return 0;
}

static int get_dev_blocks(int* fd)
{
    unsigned long long blocks = 0;
    if (ioctl(*fd, BLKGETSIZE, &blocks) == -1)
    {
        perror("ioctl");
        return -1;
    }
    return blocks;
}


static int zero_disk(int fd)
{

}

