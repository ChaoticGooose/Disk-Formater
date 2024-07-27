#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <bsd/stdlib.h>
#include <limits.h>
#include "../lib/include/fat.h"

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
    char* filesystemSubtype = NULL; // Filesystem subtype
    char* clusterSize = NULL; // Cluster size
    char *label = NULL; // Filesystem label

    bool quick = false;
    while ((opt = getopt(argc, argv, "C:F:L:q")) != -1)
    {
        switch (opt)
        {
            case 'C': /* Cluster size */
                clusterSize = optarg;
                break;
            case 'F': /* Filesystem subtype */
                filesystemSubtype = optarg;
                break;
            case 'L': /* Filesystem label */
                label = optarg;
                break;
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

    /* Select the filesystem */
    if (strcmp(filesystem, "fat") == 0) {
            // fs type validation
            if (filesystemSubtype == NULL) {
                fprintf(stderr, "No FAT type provided\n");
                exit(EXIT_FAILURE);
            }
            int type = strtol(filesystemSubtype, NULL, 10);
            if (type != 12 && type != 16 && type != 32) {
                fprintf(stderr, "Invalid FAT type\n");
                exit(EXIT_FAILURE);
            }
            
            // label validation
            if (label == NULL) {
                printf("No label provided, using default\n");
                label = "NO NAME\0\0\0\0";
            } else {
                unsigned long len = strlen(label);
                if (len > 11) {
                    fprintf(stderr, "Label too long\n");
                    exit(EXIT_FAILURE);
                }
                // pad with null characters `if label is less than 11 characters
                if (len < 11) {
                    for (int i = len; i < 11; i++) {
                        label[i] = '\0';
                    }
                }
            }

            // cluster size validation
            if (clusterSize == NULL) {
                fprintf(stderr, "No cluster size provided\n");
                exit(EXIT_FAILURE);
            }
            char* err = NULL;
            int cluster = strtol(clusterSize, &err, 10);
            if (err == NULL) {
                fprintf(stderr, "Invalid cluster size\n");
                exit(EXIT_FAILURE);
            }

            // get number of sectors proceeding the target partition
            int procSectors = 0;
            FILE *disk = fopen(disk_identifier, "r");

            fat_format(disk_identifier, type, cluster, label, quick);
    }
}
