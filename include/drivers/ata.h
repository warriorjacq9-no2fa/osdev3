#ifndef D_ATA_H
#define D_ATA_H

#define ATA_DATA    0x1F0
#define ATA_ERR     0x1F1
#define ATA_FEAT    0x1F1
#define ATA_SECS    0x1F2
#define ATA_LBALOW  0x1F3
#define ATA_LBAMID  0x1F4
#define ATA_LBAHIGH 0x1F5
#define ATA_DRIVE   0x1F6
#define ATA_STATUS  0x1F7
#define ATA_CMD     0x1F7

#define ATA_ALTSTAT 0x3F6
#define ATA_CTRL    0x3F6

enum ata_disk {
    ATA_IDE,
    ATA_ATAPIO,
    ATA_SATA
};

int ata_init();
void ata_read(uint32_t lba, uint8_t sectors, uint16_t* buf);
void ata_write(uint32_t lba, uint8_t sectors, uint16_t* buffer);

#endif