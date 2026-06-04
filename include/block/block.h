#ifndef BLOCK_H
#define BLOCK_H

#include <drivers/drivers.h>
#include <stddef.h>

typedef struct block_ops blkops_t;
typedef struct block_device blkdev_t;

struct block_ops {
    int (*read)(blkdev_t* dev, void* buf, size_t lba, size_t count);
    int (*write)(blkdev_t* dev, const void* buf, size_t lba, size_t count);
};

struct block_device {
    blkops_t* ops;
    driver_t* driver;
    char* name; // FS-mountable name, e.g. hda1 or nvme0n1p2
    unsigned int id; // Driver-specific

    size_t block_size;
    size_t nblocks;
};

int add_blkdev(blkdev_t* dev);

blkdev_t* default_blkdev();

#endif