#include <drivers/ata.h>
#include <kernel/kmalloc.h>
#include <kernel/klog.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <io.h>

#define ATA_TIMEOUT      100000
#define ATA_SECTOR_SIZE  512
#define ATA_MAX_SECTORS  255

#define ATA_STATUS_ERR   0x01
#define ATA_STATUS_DRQ   0x08
#define ATA_STATUS_DF    0x20
#define ATA_STATUS_BSY   0x80

static bool drive_present = false;

/* ============================================================
 * 400ns delay
 * ============================================================ */
static inline void ata_delay(void) {
    inb(ATA_STATUS);
    inb(ATA_STATUS);
    inb(ATA_STATUS);
    inb(ATA_STATUS);
}

/* ============================================================
 * Wait until BSY clears
 * ============================================================ */
static bool ata_wait_not_busy(void) {
    for (int i = 0; i < ATA_TIMEOUT; i++) {
        uint8_t s = inb(ATA_STATUS);

        if (!(s & ATA_STATUS_BSY))
            return true;
    }

    return false;
}

/* ============================================================
 * Poll device for PIO transfer readiness
 *
 * Requires:
 *   BSY == 0
 *   DRQ == 1
 * Fails on:
 *   ERR or DF
 * ============================================================ */
static bool ata_poll(void) {
    ata_delay();

    for (int i = 0; i < ATA_TIMEOUT; i++) {
        uint8_t s = inb(ATA_STATUS);

        if (s & ATA_STATUS_ERR)
            return false;

        if (s & ATA_STATUS_DF)
            return false;

        if (!(s & ATA_STATUS_BSY) &&
             (s & ATA_STATUS_DRQ))
        {
            return true;
        }
    }

    return false;
}

/* ============================================================
 * INIT
 * ============================================================ */
int ata_init(void) {
    drive_present = false;

    /* Select primary master */
    outb(ATA_DRIVE, 0xA0);
    ata_delay();

    /* Clear registers */
    outb(ATA_SECS,    0);
    outb(ATA_LBALOW,  0);
    outb(ATA_LBAMID,  0);
    outb(ATA_LBAHIGH, 0);

    /* IDENTIFY */
    outb(ATA_CMD, 0xEC);

    uint8_t status = inb(ATA_STATUS);

    /* No device present */
    if (status == 0) {
        kprintf(LOG_ERR, "ata", "No ATA device found\r\n");
        return -1;
    }

    /* Wait for device */
    if (!ata_poll()) {
        kprintf(LOG_ERR, "ata", "IDENTIFY failed\r\n");
        return -1;
    }

    uint16_t identity[256];

    for (int i = 0; i < 256; i++) {
        identity[i] = inw(ATA_DATA);
    }

    drive_present = true;

    uint32_t sectors =
        ((uint32_t)identity[61] << 16) |
        identity[60];

    kprintf(
        LOG_INFO,
        "ata",
        "Drive initialized, %u sectors\r\n",
        sectors
    );

    return 0;
}

/* ============================================================
 * READ (PIO28)
 * ============================================================ */
int ata_read(void* buf, size_t seek, size_t size) {
    if (!drive_present)
        return 1;

    if (size == 0)
        return 0;

    uint32_t offset  = seek % ATA_SECTOR_SIZE;
    uint32_t lba     = seek / ATA_SECTOR_SIZE;
    uint32_t sectors =
        (offset + size + ATA_SECTOR_SIZE - 1)
        / ATA_SECTOR_SIZE;

    if (sectors == 0 || sectors > ATA_MAX_SECTORS) {
        kprintf(LOG_ERR, "ata", "Invalid sector count\r\n");
        return 1;
    }

    uint8_t* dbuf =
        (uint8_t*)kmalloc(sectors * ATA_SECTOR_SIZE, 0);

    if (!dbuf)
        return 1;

    /* Wait until device idle */
    if (!ata_wait_not_busy()) {
        kprintf(LOG_ERR, "ata", "Drive busy\r\n");
        kfree(dbuf);
        return 1;
    }

    /* Select drive + high LBA nibble */
    outb(ATA_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));
    ata_delay();

    /* Program registers */
    outb(ATA_SECS,    (uint8_t)sectors);
    outb(ATA_LBALOW,  (uint8_t)(lba));
    outb(ATA_LBAMID,  (uint8_t)(lba >> 8));
    outb(ATA_LBAHIGH, (uint8_t)(lba >> 16));

    /* READ SECTORS */
    outb(ATA_CMD, 0x20);

    for (uint32_t s = 0; s < sectors; s++) {

        /* Wait for this sector */
        if (!ata_poll()) {
            kprintf(LOG_ERR, "ata", "Read error\r\n");
            kfree(dbuf);
            return 1;
        }

        /* Read 256 words = 512 bytes */
        uint16_t* ptr =
            (uint16_t*)(dbuf + (s * ATA_SECTOR_SIZE));

        for (int i = 0; i < 256; i++) {
            ptr[i] = inw(ATA_DATA);
        }

        /*
         * Required synchronization delay between sectors
         */
        ata_delay();
    }

    memcpy(buf, dbuf + offset, size);

    kfree(dbuf);
    return 0;
}

/* ============================================================
 * WRITE (PIO28)
 * ============================================================ */
int ata_write(uint32_t lba,
              uint8_t sectors,
              const uint16_t* buffer)
{
    if (!drive_present)
        return 1;

    if (sectors == 0 || sectors > ATA_MAX_SECTORS) {
        kprintf(LOG_ERR, "ata", "Invalid sector count\r\n");
        return 1;
    }

    if (!ata_wait_not_busy()) {
        kprintf(LOG_ERR, "ata", "Drive busy\r\n");
        return 1;
    }

    outb(ATA_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));
    ata_delay();

    outb(ATA_SECS,    sectors);
    outb(ATA_LBALOW,  (uint8_t)(lba));
    outb(ATA_LBAMID,  (uint8_t)(lba >> 8));
    outb(ATA_LBAHIGH, (uint8_t)(lba >> 16));

    outb(ATA_CMD, 0x30);

    for (uint32_t s = 0; s < sectors; s++) {

        if (!ata_poll()) {
            kprintf(LOG_ERR, "ata", "Write error\r\n");
            return 1;
        }

        const uint16_t* ptr = buffer + (s * 256);

        for (int i = 0; i < 256; i++) {
            outw(ATA_DATA, ptr[i]);
        }

        ata_delay();
    }

    /* Flush cache */
    outb(ATA_CMD, 0xE7);

    if (!ata_wait_not_busy()) {
        kprintf(LOG_ERR, "ata", "Flush failed\r\n");
        return 1;
    }

    kprintf(LOG_INFO, "ata", "Write complete\r\n");

    return 0;
}