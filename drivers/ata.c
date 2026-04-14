#include <drivers/ata.h>
#include <kernel/klog.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <io.h>

#define ATA_TIMEOUT 100000

static bool drive_present = false;

/* 400ns delay */
static void ata_delay() {
    inb(ATA_STATUS);
    inb(ATA_STATUS);
    inb(ATA_STATUS);
    inb(ATA_STATUS);
}

static bool ata_wait_busy() {
    for (int i = 0; i < ATA_TIMEOUT; i++) {
        if (!(inb(ATA_STATUS) & 0x80)) return true;
    }
    return false;
}

static bool ata_wait_drq() {
    for (int i = 0; i < ATA_TIMEOUT; i++) {
        uint8_t s = inb(ATA_STATUS);

        if (s & 0x08) return true;   // DRQ
        if (s & 0x01) return false;  // ERR
    }
    return false;
}

/* ================== INIT ================== */

int ata_init() {
    drive_present = false;

    outb(ATA_DRIVE, 0xA0); // master
    ata_delay();

    outb(ATA_SECS, 0);
    outb(ATA_LBALOW, 0);
    outb(ATA_LBAMID, 0);
    outb(ATA_LBAHIGH, 0);

    outb(ATA_CMD, 0xEC); // IDENTIFY

    uint8_t status = inb(ATA_STATUS);
    if (status == 0) {
        kprintf(LOG_ERR, "ata", "No drive found\n");
        return -1;
    }

    if (!ata_wait_busy()) {
        kprintf(LOG_ERR, "ata", "Timeout waiting for BSY clear\n");
        return -1;
    }

    uint8_t mid  = inb(ATA_LBAMID);
    uint8_t high = inb(ATA_LBAHIGH);

    if (mid || high) {
        uint16_t sig = (high << 8) | mid;

        if (sig == 0xEB14) {
            kprintf(LOG_INFO, "ata", "ATAPI drive detected\n");
            return 1;
        }

        if (sig == 0xC33C) {
            kprintf(LOG_INFO, "ata", "SATA drive detected\n");
            return 2;
        }

        kprintf(LOG_ERR, "ata", "Unknown drive signature\n");
        return -1;
    }

    if (!ata_wait_drq()) {
        kprintf(LOG_ERR, "ata", "IDENTIFY failed\n");
        return -1;
    }

    for (int i = 0; i < 256; i++) {
        inw(ATA_DATA);
    }

    drive_present = true;
    kprintf(LOG_INFO, "ata", "Drive initialized\n");

    return 0;
}

/* ================== READ ================== */

void ata_read(uint32_t lba, uint8_t sectors, uint16_t* buf) {
    if (!drive_present) {
        memset(buf, 0, sectors * 512);
        return;
    }

    if (!ata_wait_busy()) return;

    outb(ATA_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));
    ata_delay();

    outb(ATA_SECS, sectors);
    outb(ATA_LBALOW,  lba & 0xFF);
    outb(ATA_LBAMID,  (lba >> 8) & 0xFF);
    outb(ATA_LBAHIGH, (lba >> 16) & 0xFF);

    outb(ATA_CMD, 0x20); // READ PIO

    for (int s = 0; s < sectors; s++) {
        if (!ata_wait_busy() || !ata_wait_drq()) {
            kprintf(LOG_ERR, "ata", "Read error\n");
            return;
        }

        for (int i = 0; i < 256; i++) {
            buf[s * 256 + i] = inw(ATA_DATA);
        }
    }
}

/* ================== WRITE ================== */

void ata_write(uint32_t lba, uint8_t sectors, uint16_t* buffer) {
    if (!drive_present) return;

    if (!ata_wait_busy()) return;

    outb(ATA_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));
    ata_delay();

    outb(ATA_SECS, sectors);
    outb(ATA_LBALOW,  lba & 0xFF);
    outb(ATA_LBAMID,  (lba >> 8) & 0xFF);
    outb(ATA_LBAHIGH, (lba >> 16) & 0xFF);

    outb(ATA_CMD, 0x30); // WRITE PIO

    for (int s = 0; s < sectors; s++) {
        if (!ata_wait_busy() || !ata_wait_drq()) {
            kprintf(LOG_ERR, "ata", "Write error\n");
            return;
        }

        for (int i = 0; i < 256; i++) {
            outw(ATA_DATA, buffer[s * 256 + i]);
        }
    }

    /* Flush cache */
    outb(ATA_CMD, 0xE7);
    ata_wait_busy();
}