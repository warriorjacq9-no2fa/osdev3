#include <block/block.h>
#include <kernel/dllist.h>
#include <kernel/initcall.h>
#include <kernel/klog.h>

void _blkdev_init();

static initcall_t blkdev_init __initcall_3 = _blkdev_init;

static dllist_t* devices;
static size_t dev_count;

void _blkdev_init() {
    devices = dllist_create();
    dev_count = 0;
}

int add_blkdev(blkdev_t* dev) {
    dllist_append(devices, dev);
    dev_count++;
    kprintf(LOG_INFO, "block", "Device %s (%04X) registered, size=%lu count=%lu\r\n",
        dev->name, dev->id, dev->block_size, dev->nblocks
    );
    return 0;
}

blkdev_t* default_blkdev() {
    blkdev_t* dev = dllist_get(devices, 0);
    return dev;
}