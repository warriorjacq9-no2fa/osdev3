#ifndef DRIVER_H
#define DRIVER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef enum {
    BUS_TYPE_PCI = 1,
    BUS_TYPE_COUNT
} bus_type_t;

typedef struct {
    uint16_t vid;
    uint16_t did;
    uint16_t class_code;
    uint16_t subclass;
} pci_device_id_t;

typedef struct {
    char hid[9];
    char uid[9];
} acpi_device_id_t;

typedef struct {
    char eisa_id[8];
} eisa_device_id_t;

typedef struct {
    bus_type_t bus_type;
    union {
        pci_device_id_t  pci;
        acpi_device_id_t acpi;
        eisa_device_id_t eisa;
    };
} device_id_t;

#define DEVICE_ID_TABLE_END  { .bus_type = BUS_TYPE_COUNT }

/*
 * Opaque handle the bus layer fills in when a device is published.
 * Drivers treat this as read-only; the bus layer owns allocation.
 */
typedef struct device device_t;
typedef struct driver driver_t;

struct device {
    device_id_t id;
    driver_t* driver;
};

typedef enum {
    PROBE_OK = 0,       /* driver claimed the device         */
    PROBE_SKIP = 1,     /* driver doesn't want this device   */
    PROBE_ERROR = -1,   /* hard error during probe           */
} probe_result_t;

struct driver {
    const char          *name;        /* e.g. "e1000"                         */
    device_id_t         *id_table;    /* NULL-terminated with DEVICE_ID_TABLE_END */

    /*
     * probe()  - device was matched; driver should initialize it.
     *            matched_id points to the id_table entry that fired.
     *            Returns PROBE_OK to claim, PROBE_SKIP to pass.
     */
    probe_result_t (*probe)(device_t *dev);

    /*
     * remove() - device is going away (hot-unplug, shutdown).
     *            Driver must release all resources acquired in probe().
     */
    void           (*remove)(device_t *dev);
};

/*
 * driver_register()
 *   Call during driver init (or __attribute__((constructor)) if you later
 *   add a .drivers ELF section).  Safe to call before any bus is ready.
 */
void driver_register(driver_t *drv);
void driver_unregister(driver_t *drv);

/*
 * device_publish()
 *   Called by a bus layer when it enumerates a new device.
 *   The core walks all registered drivers, tests their id_table against
 *   the bus-specific fields, and calls probe() on the first match.
 *   `dev` must remain valid until device_unpublish().
 */
void device_publish(device_t *dev);
void device_unpublish(device_t *dev);

#endif