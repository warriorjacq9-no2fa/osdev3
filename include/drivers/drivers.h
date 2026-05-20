#ifndef DRIVERS_H
#define DRIVERS_H

#include <stddef.h>
#include <stdint.h>

enum bus_type {
    BUS_NONE,
    BUS_PCI,
    BUS_ISA,
    BUS_SMBUS,
    BUS_FIXED
};

typedef struct device_id {
    enum bus_type bus;

    union {
        struct {
            uint16_t vid, did;
        } pci;

        struct {
            uint16_t io_base;
            uint8_t irq;
        } isa;

        struct {
            uint8_t addr;
        } smbus;
    };
} device_id_t;

typedef struct device {
    device_id_t id;
    union {
        struct {
            uint32_t 
        }
    }
}

typedef struct driver {
    const char* name;
    device_id_t* id_table;
    size_t id_len;
} driver_t;

#endif