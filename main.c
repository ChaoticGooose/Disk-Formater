#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include "lib/include/fat.h"

// format <filesystem> <disk_identifier> [-q]
int main(int argc, char *argv[]) {
    /*
        Parse command line arguments
        Options:
            -q : quick format
        Parameters:
            filesystem : filesystem to format
            disk_identifier : disk to format
    */
    int opt; // Loop variable
    bool quick = false;
    while ((opt = getopt(argc, argv, "q")) != -1)
    {
        switch (opt)
        {
            case 'q': /* Quick format */
                quick = true;
                break;
            default: /* '?' */
                fprintf(stderr, "Usage: %s [-q] <filesystem> <disk_identifier>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    if (optind >= argc || argc - optind != 2) { // Ensure that there are exactly 2 positional arguments, format and disk_identifier
        fprintf(stderr, "%s: invalid usage\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *filesystem = argv[optind];
    char *disk_identifier = argv[optind + 1];

    printf("FS: %s\n", filesystem);
    printf("Disk: %s\n", disk_identifier);

    fat_format(disk_identifier, quick);
}
