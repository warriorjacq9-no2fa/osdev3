#include <drivers/ata.h>
#include <kernel/kmalloc.h>
#include <kernel/klog.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <io.h>

#define ATA_TIMEOUT 100000
#define ATA_SECTOR_SIZE 512
#define ATA_MAX_SECTORS 256

#define ATA_STATUS_BSY  0x80
#define ATA_STATUS_DRQ  0x08
#define ATA_STATUS_ERR  0x01
#define ATA_STATUS_DF   0x20

static bool drive_present = false;

/* 400ns delay */
static void ata_delay() {
    inb(ATA_STATUS);
    inb(ATA_STATUS);
    inb(ATA_STATUS);
    inb(ATA_STATUS);
}

/* Wait until BSY clears and optionally check error */
static bool ata_wait_ready() {
    for (int i = 0; i < ATA_TIMEOUT; i++) {
        uint8_t s = inb(ATA_STATUS);

        if (s & ATA_STATUS_ERR) return false;
        if (s & ATA_STATUS_DF)  return false;

        if (!(s & ATA_STATUS_BSY))
            return true;
    }
    return false;
}

/* Wait for DRQ */
static bool ata_wait_drq() {
    for (int i = 0; i < ATA_TIMEOUT; i++) {
        uint8_t s = inb(ATA_STATUS);

        if (s & ATA_STATUS_ERR) return false;
        if (s & ATA_STATUS_DF)  return false;

        if (s & ATA_STATUS_DRQ)
            return true;
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

    if (!ata_wait_ready()) {
        kprintf(LOG_ERR, "ata", "IDENTIFY: device not ready\r\n");
        return -1;
    }

    uint8_t status = inb(ATA_STATUS);
    if (status == 0) {
        kprintf(LOG_ERR, "ata", "No ATA device found\r\n");
        return -1;
    }

    if (!ata_wait_drq()) {
        kprintf(LOG_ERR, "ata", "IDENTIFY: DRQ not set\r\n");
        return -1;
    }

    uint16_t identity[256];
    for (int i = 0; i < 256; i++) {
        identity[i] = inw(ATA_DATA);
    }

    drive_present = true;
    kprintf(LOG_INFO, "ata", "Drive initialized, %u sectors\r\n", identity[60] | (identity[61] << 16));

    return 0;
}

/* ================== READ ================== */

int ata_read(void* buf, size_t seek, size_t size) {
    if (!drive_present) return 1;

    uint32_t offset = seek % ATA_SECTOR_SIZE;
    uint32_t lba = seek / ATA_SECTOR_SIZE;
    uint32_t sectors = (offset + size + ATA_SECTOR_SIZE - 1) / ATA_SECTOR_SIZE;

    if (sectors > ATA_MAX_SECTORS) {
        kprintf(LOG_ERR, "ata", "Read exceeds max sector limit\r\n");
        return 1;
    }

    uint8_t *dbuf = (uint8_t*)kmalloc(sectors * ATA_SECTOR_SIZE, 0);
    if (!dbuf) return 1;

    if (!ata_wait_ready()) {
        kprintf(LOG_ERR, "ata", "Read: device busy\r\n");
        kfree(dbuf);
        return 1;
    }

    outb(ATA_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));
    ata_delay();

    outb(ATA_SECS, sectors);
    outb(ATA_LBALOW,  lba & 0xFF);
    outb(ATA_LBAMID,  (lba >> 8) & 0xFF);
    outb(ATA_LBAHIGH, (lba >> 16) & 0xFF);

    outb(ATA_CMD, 0x20); // READ PIO

    for (uint32_t s = 0; s < sectors; s++) {
        if (!ata_wait_ready() || !ata_wait_drq()) {
            kprintf(LOG_ERR, "ata", "Read error\r\n");
            kfree(dbuf);
            return 1;
        }

        for (int i = 0; i < 256; i++) {
            ((uint16_t*)dbuf)[s * 256 + i] = inw(ATA_DATA);
        }
    }

    memcpy(buf, dbuf + offset, size);

    kfree(dbuf);
    return 0;
}

/* ================== WRITE ================== */

int ata_write(uint32_t lba, uint8_t sectors, const uint16_t* buffer) {
    if (!drive_present) return 1;
    if (sectors > ATA_MAX_SECTORS) {
        kprintf(LOG_ERR, "ata", "Write exceeds max sector limit\r\n");
        return 1;
    }

    if (!ata_wait_ready()) {
        kprintf(LOG_ERR, "ata", "Write: device busy\r\n");
        return 1;
    }

    outb(ATA_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));
    ata_delay();

    outb(ATA_SECS, sectors);
    outb(ATA_LBALOW,  lba & 0xFF);
    outb(ATA_LBAMID,  (lba >> 8) & 0xFF);
    outb(ATA_LBAHIGH, (lba >> 16) & 0xFF);

    outb(ATA_CMD, 0x30); // WRITE PIO

    for (int s = 0; s < sectors; s++) {
        if (!ata_wait_ready() || !ata_wait_drq()) {
            kprintf(LOG_ERR, "ata", "Write error\r\n");
            return 1;
        }

        for (int i = 0; i < 256; i++) {
            outw(ATA_DATA, buffer[s * 256 + i]);
        }
    }

    /* Flush cache */
    outb(ATA_CMD, 0xE7);

    if (!ata_wait_ready()) {
        kprintf(LOG_ERR, "ata", "Flush failed\r\n");
        return 1;
    }

    kprintf(LOG_INFO, "ata", "Write complete\r\n");
    return 0;
}