#include <drivers/drivers.h>
#include <drivers/pci.h>
#include <kernel/klog.h>
#include <kernel/initcall.h>
#include <string.h>
#include <stdio.h>

void _driver_init();
static initcall_t driver_init __initcall_2 = _driver_init;

static driver_t* drivers[256];
static size_t d_idx;

void _driver_init() {
    d_idx = 0;
}

void driver_register(driver_t* drv) {
    drivers[d_idx++] = drv;
}

void driver_unregister(driver_t* drv) {
    for(int i = 0; i < d_idx; i++) {
        if(drivers[i] == drv) {
            drivers[i] = 0;
            memmove(drivers[i], drivers[i + 1], d_idx - i);
        }
    }
}

void device_publish(device_t *dev) {
    switch(dev->id.bus_type) {
        case BUS_TYPE_PCI:
            kprintf(LOG_INFO, "drivers", "PCI device published: %u.%u %04x:%04x\r\n",
                dev->id.pci.class_code,
                dev->id.pci.subclass,
                dev->id.pci.vid,
                dev->id.pci.did
            );
        default:
            break;
    }
    for(size_t i = 0; i < d_idx; i++) {
        size_t j = 0;
        device_id_t *id;
        while((id = &drivers[i]->id_table[j])->bus_type != BUS_TYPE_COUNT) {
            if(dev->id.bus_type != id->bus_type) goto next;
            switch(dev->id.bus_type) {
                case BUS_TYPE_PCI:
                    if(id->pci.vid != 0xFFFF && id->pci.vid != dev->id.pci.vid) goto next;
                    if(id->pci.did != 0xFFFF && id->pci.did != dev->id.pci.did) goto next;
                    if(id->pci.class_code != 0xFFFF && id->pci.class_code != dev->id.pci.class_code) goto next;
                    if(id->pci.subclass != 0xFFFF && id->pci.subclass != dev->id.pci.subclass) goto next;
                    if(drivers[i]->probe(dev) != PROBE_OK) goto next;
                    kprintf(LOG_INFO, "drivers", "Device claimed by driver %s\r\n", drivers[i]->name);
                    return;
                default:
                    return;
            }
next:
            j++;
        }
    }
}

void device_unpublish(device_t* dev) {}