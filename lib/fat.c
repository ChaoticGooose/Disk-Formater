#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <sys/ioctl.h>

static int get_dev_blocks(int* fd);
static int zero_disk(int* fd);

int fat_format(char* target, bool quick)
{
    int fd = open(target, O_WRONLY);
    if (fd == -1)
    {
        perror("open");
        return 1;
    }

    zero_disk(&fd);

//    int blocks = get_dev_blocks(&fd);
//    printf("blocks: %d\n", blocks);

    return 0;
}

/* Get number of sectors on fd
 * https://man7.org/linux/man-pages/man4/sd.4.html/
*/
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

static int zero_disk(int* fd)
{
    char* zeros = calloc(1, 512); // 512 byte sector size, Should really be checking how large the sectors are
    ssize_t written, total = 0;
    do {
        total += written = write(*fd, zeros, 512); // Write 0s to each sector
    } while (written == 512);

    free(zeros);
    printf("Bytes Written: %ld", total);
    return 0;
}

