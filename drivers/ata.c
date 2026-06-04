/**
 * The general idea is that the ATA driver presents to the kernel
 * a flat array (or similar structure) of disks, but in reality
 * each could be controlled by a separate controller on a
 * separate channel.
 * 
 * Each drive ID will be structured so that id / 4 is the controller
 * index, (id / 2) % 2 is the channel number, and id % 2 is the
 * master/slave selector
 */

#include <drivers/ata.h>
#include <drivers/drivers.h>
#include <drivers/pci.h>
#include <kernel/kmalloc.h>
#include <kernel/klog.h>
#include <kernel/initcall.h>
#include <kernel/dllist.h>
#include <block/block.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <io.h>

#define ATA_TIMEOUT      2500000
#define ATA_SECTOR_SIZE  512
#define ATA_MAX_SECTORS  255

#define ATA_STATUS_ERR   0x01
#define ATA_STATUS_DRQ   0x08
#define ATA_STATUS_DF    0x20
#define ATA_STATUS_BSY   0x80

void _ata_init();
static initcall_t ata_init __initcall_3 = _ata_init;

static driver_t driver;
static blkops_t ops;

static dllist_t* controllers;
static size_t num_controllers;

static inline void ata_delay(uint16_t base) {
    inb(base + ATA_STATUS);
    inb(base + ATA_STATUS);
    inb(base + ATA_STATUS);
    inb(base + ATA_STATUS);
}

static bool ata_wait_not_busy(uint16_t base) {
    for (int i = 0; i < ATA_TIMEOUT; i++) {
        uint8_t s = inb(base + ATA_STATUS);

        if (!(s & ATA_STATUS_BSY))
            return true;
    }

    return false;
}

static bool ata_poll(uint16_t base) {
    ata_delay(base);

    for (int i = 0; i < ATA_TIMEOUT; i++) {
        uint8_t s = inb(base + ATA_STATUS);

        if (s & ATA_STATUS_ERR) {
            kprintf(LOG_WARN, "ata", "ERR %08x\r\n", s);
            return false;
        }

        if (s & ATA_STATUS_DF) {
            kprintf(LOG_WARN, "ata", "DF %08x\r\n", s);
            return false;
        }

        if (!(s & ATA_STATUS_BSY) &&
             (s & ATA_STATUS_DRQ))
        {
            return true;
        }
    }

    kprintf(LOG_WARN, "ata", "Timeout\r\n");
    return false;
}

int ata_identify(uint16_t base, uint8_t disk, uint16_t* identity) {
    outb(base + ATA_DRIVE, disk);
    ata_delay(base);

    outb(base + ATA_SECS,    0);
    outb(base + ATA_LBALOW,  0);
    outb(base + ATA_LBAMID,  0);
    outb(base + ATA_LBAHIGH, 0);

    /* IDENTIFY */
    outb(base + ATA_CMD, 0xEC);

    uint8_t status = inb(base + ATA_STATUS);

    /* No device present */
    if (status == 0) {
        return -2;
    }

    /* Wait for device */
    if (!ata_poll(base)) {
        kprintf(LOG_WARN, "ata", "IDENTIFY failed for disk %03X:%02X\r\n", base, disk);
        return -1;
    }

    for (int i = 0; i < 256; i++) {
        identity[i] = inw(base + ATA_DATA);
    }

    return 0;
}

probe_result_t ata_probe(device_t* dev) {
    if(dev->id.bus_type != BUS_TYPE_PCI) return PROBE_SKIP;
    pci_device_t* device = pci_device(dev);

    ata_ctrl_t* controller = kmalloc(sizeof(ata_ctrl_t), 0);
    if(controller == NULL) return PROBE_ERROR;
    memset(controller, 0, sizeof(ata_ctrl_t));
    num_controllers++;

    for(int channel = 0; channel < 2; channel++) {
        uint16_t d_base;
        if(device->prog_if & 0x01) { // PCI native controller 
            if(!device->bar[channel * 2]) {
                kprintf(LOG_WARN, "ata", "Invalid BAR for channel %d\r\n", channel);
                continue;
            }
            d_base = device->bar[channel * 2];
            kprintf(LOG_INFO, "ata", "Channel %d at %08X\r\n", channel, d_base);
        } else { // Compat mode (0x1F0-0x1F7, 0x170-0x177)
            d_base = (channel == 0) ? 0x1F0 : 0x170;
        }
        uint8_t status = inb(d_base + ATA_STATUS);
        if(status == 0xFF) {
            kprintf(LOG_INFO, "ata", "Channel %d not present\r\n", channel);
            continue;
        }
        controller->base[channel] = d_base;

        for(int drive = 0; drive < 2; drive++) {
            ata_disk_t* disk = &controller->disks[channel * 2 + drive];
            blkdev_t* dev = kmalloc(sizeof(blkdev_t), 0);

            int res;
            uint16_t* identity = kmalloc(256 * sizeof(uint16_t), 0);

            if((res = ata_identify(
                d_base, 
                (drive == 0) ? 0xA0 : 0xB0,
                identity
            )) < 0) {
                continue;
            }

            if((identity[106] & 0xD000) == 0x5000) {
                dev->block_size = *(uint32_t*)&identity[117];
            } else dev->block_size = 512;

            disk->present = true;
            disk->lba48 = identity[86] & 0x0400;
            if(disk->lba48) {
                dev->nblocks = *(uint64_t*)&identity[100];
            } else {
                dev->nblocks = *(uint64_t*)&identity[60];
            }
            
            disk->size = dev->nblocks * dev->block_size;
            disk->dma = identity[49] & 0x0100;
            char* name = kmalloc(41, 0);
            for(int i = 0; i < 20; i++) {
                name[i * 2 + 1] = identity[27 + i] & 0xFF;
                name[i * 2] = identity[27 + i] >> 8;
            }
            name[40] = '\0';
            disk->name = name;
            if(disk->size < (1 << 30))
                kprintf(LOG_INFO, "ata", "Found disk %s with size %dMB\r\n", disk->name, disk->size / (1 << 20));
            else
                kprintf(LOG_INFO, "ata", "Found disk %s with size %dGB\r\n", disk->name, disk->size / (1 << 30));

            dev->id = ((num_controllers - 1) << 2) | (channel << 1) | drive;

            dev->name = kmalloc(8, 0);
            memset(dev->name, 0, 8);
            sprintf(dev->name, "ide%d", dev->id);
            
            dev->driver = &driver;
            dev->ops = &ops;

            add_blkdev(dev);
        }
    }

    dllist_append(controllers, controller);

    return PROBE_OK;
}

int ata_read(blkdev_t* dev, void* buf, size_t lba, size_t count) {
    if (count == 0)
        return 0;
    else if (count > dev->nblocks) {
        kprintf(LOG_ERR, "ata", "Invalid sector count\r\n");
        return 1;
    }

    if (buf == NULL)
        return 1;

    uint8_t drive = (dev->id & 0x01) << 4;

    ata_ctrl_t* controller = dllist_get(controllers, dev->id >> 2);
    uint16_t base = controller->base[(dev->id >> 1) & 0x01];
    if(!base) return 1;

    /* Wait until device idle */
    if (!ata_wait_not_busy(base)) {
        kprintf(LOG_WARN, "ata", "Drive busy\r\n");
        return 1;
    }

    /* Select drive + high LBA nibble */
    outb(base + ATA_DRIVE, 0xE0 | ((lba >> 24) & 0x0F) | drive);
    ata_delay(base);

    /* Program registers */
    outb(base + ATA_SECS,    (uint8_t)count);
    outb(base + ATA_LBALOW,  (uint8_t)(lba));
    outb(base + ATA_LBAMID,  (uint8_t)(lba >> 8));
    outb(base + ATA_LBAHIGH, (uint8_t)(lba >> 16));

    /* READ SECTORS */
    outb(base + ATA_CMD, 0x20);

    for (uint32_t s = 0; s < count; s++) {

        /* Wait for this sector */
        if (!ata_poll(base)) {
            kprintf(LOG_ERR, "ata", "Read error\r\n");
            return 1;
        }

        /* Read 256 words = 512 bytes */
        uint16_t* ptr =
            (uint16_t*)((uint8_t*)buf + (s * dev->block_size));

        for (int i = 0; i < 256; i++) {
            ptr[i] = inw(base + ATA_DATA);
        }

        /*
         * Required synchronization delay between sectors
         */
        ata_delay(base);
    }

    return 0;
}

int ata_write(blkdev_t* dev, const void* buf, size_t lba, size_t count) {
    uint8_t drive = (dev->id & 0x01) << 4;

    ata_ctrl_t* controller = dllist_get(controllers, dev->id >> 2);
    uint16_t base = controller->base[(dev->id >> 1) & 0x01];
    if(!base) return 1;

    if(count == 0)
        return 0;
    else if(count > dev->nblocks) {
        kprintf(LOG_ERR, "ata", "Invalid sector count\r\n");
        return 1;
    }

    if (!ata_wait_not_busy(base)) {
        kprintf(LOG_WARN, "ata", "Drive busy\r\n");
        return 1;
    }

    outb(base + ATA_DRIVE, 0xE0 | ((lba >> 24) & 0x0F) | drive);
    ata_delay(base);

    outb(base + ATA_SECS,    count);
    outb(base + ATA_LBALOW,  (uint8_t)(lba));
    outb(base + ATA_LBAMID,  (uint8_t)(lba >> 8));
    outb(base + ATA_LBAHIGH, (uint8_t)(lba >> 16));

    outb(base + ATA_CMD, 0x30);

    for (uint32_t s = 0; s < count; s++) {

        if (!ata_poll(base)) {
            kprintf(LOG_ERR, "ata", "Write error\r\n");
            return 1;
        }

        const uint16_t* ptr = (uint16_t*)(buf + (s * ATA_SECTOR_SIZE));

        for (int i = 0; i < 256; i++) {
            outw(base + ATA_DATA, ptr[i]);
        }

        ata_delay(base);
    }

    /* Flush cache */
    outb(base + ATA_CMD, 0xE7);

    if (!ata_wait_not_busy(base)) {
        kprintf(LOG_ERR, "ata", "Flush failed\r\n");
        return 1;
    }

    kprintf(LOG_INFO, "ata", "Write complete\r\n");

    return 0;
}

void _ata_init() {
    controllers = 0;

    controllers = dllist_create();

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
        .name = "ata",
        .probe = ata_probe,
        .remove = NULL
    };
    ops = (blkops_t){
        .read = ata_read,
        .write = ata_write
    };
    driver_register(&driver);
    return;
}