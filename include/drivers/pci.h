#ifndef D_PCI_H
#define D_PCI_H

#include <stdint.h>
#include <stddef.h>
#include <drivers/drivers.h>

#define PCI_CONFIG_ADDR 0xCF8
#define PCI_CONFIG_DATA 0xCFC

struct pci_config_address {
    unsigned int    off : 8;
    unsigned int    func: 3;
    unsigned int    dev : 5;
    unsigned int    bus : 8;
    unsigned int    res : 7;
    unsigned int    en  : 1; 
} __attribute__((packed));

typedef union {
    struct pci_config_address cfg;
    uint32_t val;
} __attribute__((packed)) pci_cfg_addr_t;

typedef struct pci_header_common {
    uint16_t    vid;
    uint16_t    did;
    uint16_t    command;
    uint16_t    status;
    uint8_t     revision;
    uint8_t     prog_if;
    uint8_t     subclass;
    uint8_t     class_code;
    uint8_t     cache_size;
    uint8_t     latency;
    uint8_t     header_type;
    uint8_t     bist;
} __attribute__((packed)) pci_hc_t;

#define PCR_IO      0
#define PCR_MEM     1
#define PCR_MASTER  2
#define PCR_SPECIAL 3
#define PCR_INVAL   4
#define PCR_VGASNOOP 5
#define PCR_ERROR   6
#define PCR_SERR_EN 8
#define PCR_FASTB2B 9
#define PCR_INT     10

#define PSR_INT         3
#define PSR_CAPS        4
#define PSR_66M         5
#define PSR_FASTB2B     7
#define PSR_EMPARITY    8
#define PSR_DEVSEL_L    9
#define PSR_DEVSEL_H    10
#define PSR_SIGABORT    11
#define PSR_RECVABORT   12
#define PSR_MASTERABORT 13
#define PSR_ESYSTEM     14
#define PSR_EDPARITY    15

typedef struct pci_header_0 {
    uint32_t    bar[6];
    uint32_t    cis_ptr;
    uint16_t    subsystem_vid;
    uint16_t    subsystem_id;
    uint32_t    rom_base;
    uint32_t    cap_ptr;
    uint32_t    res;
    uint8_t     int_line;
    uint8_t     int_pin;
    uint8_t     min_grant;
    uint8_t     max_latency;
} __attribute__((packed)) pci_h0_t;

typedef struct pci_header_1 {
    uint32_t    bar[2];
    uint8_t     prim_bus;
    uint8_t     sec_bus;
    uint8_t     subordinate_bus;
    uint8_t     sec_latency;
    uint8_t     io_base;
    uint8_t     io_limit;
    uint16_t    sec_status;
    uint16_t    mem_base;
    uint16_t    mem_limit;
    uint16_t    pf_base;
    uint16_t    pf_limit;
    uint32_t    pf_base_upper;
    uint32_t    pf_limit_upper;
    uint16_t    io_base_upper;
    uint16_t    io_limit_upper;
    uint32_t    cap;
    uint32_t    rom_base;
    uint8_t     int_line;
    uint8_t     int_pin;
    uint16_t    bridge_ctrl;
} __attribute__((packed)) pci_h1_t;

struct pci_bus_handle {
    uint8_t bus, dev, func;
};

typedef union {
    struct pci_bus_handle handle;
    uintptr_t val;
} pci_handle_t;

typedef struct {
    device_t    device;

    uint32_t    bar[6];
    bool        mmio[6];
    uint8_t     prog_if;
} pci_device_t;

#define pci_device(d) ((pci_device_t*)d)

void pci_enumerate();

#endif