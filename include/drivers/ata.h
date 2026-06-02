#ifndef D_ATA_H
#define D_ATA_H

#include <stdint.h>
#include <stddef.h>

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

enum ata_disk {
    ATA_IDE,
    ATA_ATAPIO,
    ATA_SATA
};

int ata_read(void* buf, size_t seek, size_t size);
int ata_write(uint32_t lba, uint8_t sectors, const uint16_t* buffer);

#endif