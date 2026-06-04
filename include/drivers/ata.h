#ifndef D_ATA_H
#define D_ATA_H

#include <block/block.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define ATA_DATA    0
#define ATA_ERR     1
#define ATA_FEAT    1
#define ATA_SECS    2
#define ATA_LBALOW  3
#define ATA_LBAMID  4
#define ATA_LBAHIGH 5
#define ATA_DRIVE   6
#define ATA_STATUS  7
#define ATA_CMD     7

typedef struct ata_disk {
    bool        present;
    char*       name;
    uint64_t    size;

    bool        lba48;
    bool        dma;
} ata_disk_t;

typedef struct ata_controller {
    uint16_t    base[2];
    uint16_t    ctrl[2];
    uint16_t    base_bus;
    
    ata_disk_t  disks[4];
} ata_ctrl_t;

int ata_read(blkdev_t* dev, void* buf, size_t lba, size_t count);
int ata_write(blkdev_t* dev, const void* buf, size_t lba, size_t count);

#endif