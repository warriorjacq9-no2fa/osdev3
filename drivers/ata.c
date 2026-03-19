#include <drivers/ata.h>
#include <kernel/klog.h>
#include <io.h>
#include <string.h>
#include <stdbool.h>


void ata_busy() {
    while(inb(ATA_STATUS) & 0x80);
}

void ata_drq() {
    while(!(inb(ATA_STATUS) & 0x08));
}

bool drive;

int ata_init() {
    drive = false;
    outb(ATA_DRIVE, 0xA0);

    outb(ATA_SECS, 0);
    outb(ATA_LBALOW, 0);
    outb(ATA_LBAMID, 0);
    outb(ATA_LBAHIGH, 0);

    outb(ATA_CMD, 0xEC);

    uint8_t status = inb(ATA_STATUS);
    if(status) {
        ata_busy();
        
        uint16_t i = inb(ATA_LBAMID);
        i |= inb(ATA_LBAHIGH) << 8;
        if(i) {
            if(i == 0xEB14) {
                kprintf(LOG_INFO, "ata", "Drive is ATAPIO\r\n");
                return ATA_ATAPIO;
            } else if(i == 0xC33C) {
                kprintf(LOG_INFO, "ata", "Drive is SATA\r\n");
                return ATA_SATA;
            } else {
                kprintf(LOG_ERR, "ata", "Error when getting status\r\n");
                return -1;
            }
        }
        ata_drq();
        if(!(inb(ATA_STATUS) & 0x01)){
            for(int i = 0; i < 256; i++) {
                inw(ATA_DATA);
            }
            drive = true;
            return 0;
        } else {
            kprintf(LOG_ERR, "ata", "Error when getting status\r\n");
        }
    }
    kprintf(LOG_ERR, "ata", "No drive found\r\n");
    return -1;
}

void ata_read(uint32_t lba, uint8_t sectors, uint16_t* buf) {
    if(!drive) {
        memset(buf, 0, sectors * 512);
        return;
    }
    ata_busy();

    outb(ATA_DRIVE, 0xE0 | ((lba >> 24) & 0x0F)); // master + LBA
    outb(ATA_SECS, sectors);

    outb(ATA_LBALOW,  (uint8_t)(lba));
    outb(ATA_LBAMID,  (uint8_t)(lba >> 8));
    outb(ATA_LBAHIGH, (uint8_t)(lba >> 16));

    outb(ATA_CMD, 0x20);

    ata_busy();
    ata_drq();

    for (int i = 0; i < sectors * 256; i++) {
        buf[i] = inw(ATA_DATA);
    }

}

void ata_write(uint32_t lba, uint8_t sectors, uint16_t* buffer) {
    ata_busy();

    outb(ATA_DRIVE, 0xE0 | ((lba >> 24) & 0x0F)); // master + LBA
    outb(ATA_SECS, 1);

    outb(ATA_LBALOW,  (uint8_t)(lba));
    outb(ATA_LBAMID,  (uint8_t)(lba >> 8));
    outb(ATA_LBAHIGH, (uint8_t)(lba >> 16));
    
    outb(ATA_CMD, 0x30);
    
    ata_busy();
    
    for (int i = 0; i < sectors * 256; i++) {
        outw(ATA_DATA, buffer[i]);
        iowait();
    }
    
    outb(ATA_CMD, 0xE7);
    ata_busy();
}