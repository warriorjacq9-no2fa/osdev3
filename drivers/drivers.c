#include <drivers.h>
#include <string.h>

static driver_t* drivers[256];
static size_t d_idx;

void driver_init() {
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
    for(size_t i = 0; i < d_idx; i++) {
        size_t j = 0;
        device_id_t id;
        while((id = drivers[i]->id_table[j]).bus_type != BUS_TYPE_COUNT) {
            bool match = true;
            if(dev->)
        }
    }
}