#include <drivers/ata.h>
#include <drivers/drivers.h>
#include <drivers/pci.h>
#include <kernel/kmalloc.h>
#include <kernel/klog.h>
#include <kernel/initcall.h>
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

void _ata_init();
static initcall_t ata_init __initcall_3 = _ata_init;

static driver_t driver;

static uintptr_t base;

static inline void ata_delay(void) {
    inb(base + ATA_STATUS);
    inb(base + ATA_STATUS);
    inb(base + ATA_STATUS);
    inb(base + ATA_STATUS);
}

static bool ata_wait_not_busy(void) {
    for (int i = 0; i < ATA_TIMEOUT; i++) {
        uint8_t s = inb(base + ATA_STATUS);

        if (!(s & ATA_STATUS_BSY))
            return true;
    }

    return false;
}

static bool ata_poll(void) {
    ata_delay();

    for (int i = 0; i < ATA_TIMEOUT; i++) {
        uint8_t s = inb(base + ATA_STATUS);

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

probe_result_t ata_probe(device_t* dev) {
    if(dev->id.bus_type != BUS_TYPE_PCI) return PROBE_SKIP;
    pci_device_t* device = pci_device(dev);

    if(device->prog_if & 0x01) { // PCI native controller
        for(int i = 0; i < 6; i++) {
            if(device->bar[i] == 0) continue;
            if(!device->mmio[i]) base = device->bar[i];
        }
    } else { // Compat mode (0x1F0-0x1F7)
        base = 0x1F0;
    }

    if(!base) {
        kprintf(LOG_WARN, "ata", "Base address not found\r\n");
        return PROBE_SKIP;
    }
    kprintf(LOG_INFO, "ata", "Successfully probed device at %08X\r\n", base);

    /* Select primary master */
    outb(base + ATA_DRIVE, 0xA0);
    ata_delay();

    /* Clear registers */
    outb(base + ATA_SECS,    0);
    outb(base + ATA_LBALOW,  0);
    outb(base + ATA_LBAMID,  0);
    outb(base + ATA_LBAHIGH, 0);

    /* IDENTIFY */
    outb(base + ATA_CMD, 0xEC);

    uint8_t status = inb(base + ATA_STATUS);

    /* No device present */
    if (status == 0) {
        kprintf(LOG_ERR, "ata", "No ATA device found\r\n");
        return PROBE_SKIP;
    }

    /* Wait for device */
    if (!ata_poll()) {
        kprintf(LOG_ERR, "ata", "IDENTIFY failed\r\n");
        return PROBE_ERROR;
    }

    uint16_t identity[256];

    for (int i = 0; i < 256; i++) {
        identity[i] = inw(base + ATA_DATA);
    }

    uint32_t sectors =
        ((uint32_t)identity[61] << 16) |
        identity[60];

    kprintf(
        LOG_INFO,
        "ata",
        "Drive initialized, %u sectors\r\n",
        sectors
    );

    return PROBE_OK;
}

void _ata_init() {
    base = 0;
    device_id_t* id_table = kmalloc(sizeof(device_id_t) * 2, 0);
    id_table[0].bus_type = BUS_TYPE_PCI;
    id_table[0].pci = (pci_device_id_t){
        .vid = 0xFFFF,
        .did = 0xFFFF,
        .class_code = 0x1,
        .subclass = 0x1
    };
    id_table[1] = (device_id_t)DEVICE_ID_TABLE_END;
    driver = (driver_t){
        .id_table = id_table,
        .name = "IDE driver",
        .probe = ata_probe,
        .remove = NULL
    };
    driver_register(&driver);
    return;
}

int ata_read(void* buf, size_t seek, size_t size) {
    if (!base)
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
    outb(base + ATA_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));
    ata_delay();

    /* Program registers */
    outb(base + ATA_SECS,    (uint8_t)sectors);
    outb(base + ATA_LBALOW,  (uint8_t)(lba));
    outb(base + ATA_LBAMID,  (uint8_t)(lba >> 8));
    outb(base + ATA_LBAHIGH, (uint8_t)(lba >> 16));

    /* READ SECTORS */
    outb(base + ATA_CMD, 0x20);

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
            ptr[i] = inw(base + ATA_DATA);
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

int ata_write(uint32_t lba,
              uint8_t sectors,
              const uint16_t* buffer)
{
    if (!base)
        return 1;

    if (sectors == 0 || sectors > ATA_MAX_SECTORS) {
        kprintf(LOG_ERR, "ata", "Invalid sector count\r\n");
        return 1;
    }

    if (!ata_wait_not_busy()) {
        kprintf(LOG_ERR, "ata", "Drive busy\r\n");
        return 1;
    }

    outb(base + ATA_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));
    ata_delay();

    outb(base + ATA_SECS,    sectors);
    outb(base + ATA_LBALOW,  (uint8_t)(lba));
    outb(base + ATA_LBAMID,  (uint8_t)(lba >> 8));
    outb(base + ATA_LBAHIGH, (uint8_t)(lba >> 16));

    outb(base + ATA_CMD, 0x30);

    for (uint32_t s = 0; s < sectors; s++) {

        if (!ata_poll()) {
            kprintf(LOG_ERR, "ata", "Write error\r\n");
            return 1;
        }

        const uint16_t* ptr = buffer + (s * 256);

        for (int i = 0; i < 256; i++) {
            outw(base + ATA_DATA, ptr[i]);
        }

        ata_delay();
    }

    /* Flush cache */
    outb(base + ATA_CMD, 0xE7);

    if (!ata_wait_not_busy()) {
        kprintf(LOG_ERR, "ata", "Flush failed\r\n");
        return 1;
    }

    kprintf(LOG_INFO, "ata", "Write complete\r\n");

    return 0;
}