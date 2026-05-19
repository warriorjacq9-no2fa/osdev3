#ifndef D_PCI_H
#define D_PCI_H

#include <stdint.h>

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
    uint16_t    devid;
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
    uint16_t    subsystem_id;
    uint16_t    subsystem_vid;
    uint32_t    rom_base;
    uint8_t     res0[3];
    uint8_t     cap_ptr;
    uint32_t    res1;
    uint8_t     max_latency;
    uint8_t     min_grant;
    uint8_t     int_pin;
    uint8_t     int_line;
} __attribute__((packed)) pci_h0_t;

typedef struct pci_header_1 {
    uint32_t    bar[2];
    uint8_t     sec_latency;
    uint8_t     subordinate_bus;
    uint8_t     sec_bus;
    uint8_t     prim_bus;
    uint16_t    sec_status;
    uint8_t     io_limit;
    uint8_t     io_base;
    uint16_t    mem_limit;
    uint16_t    mem_base;
    uint16_t    pf_limit;
    uint16_t    pf_base;
    uint32_t    pf_base_upper;
    uint32_t    pf_limit_upper;
    uint16_t    io_limit_upper;
    uint16_t    io_base_upper;
    uint8_t     res[3];
    uint8_t     cap;
    uint32_t    rom_base;
    uint16_t    bridge_ctrl;
    uint8_t     int_pin;
    uint8_t     int_line;
} __attribute__((packed)) pci_h1_t;

typedef struct pci_class_lookup {
    uint8_t class_code;
    const char* name;
} pci_clookup_t;

static const pci_clookup_t pci_classes[18][] = {
    {
        {0x00, "Unclassified device"},
        {0x01, "VGA compatible"}
    },
    {
        {0x00, "SCSI controller"},
        {0x01, "IDE controller"},
        {0x02, "Floppy disk controller"},
        {0x03, "IPI bus controller"},
        {0x04, "RAID controller"},
        {0x05, "ATA controller"},
        {0x06, "SATA controller"},
        {0x07, "SAS controller"},
        {0x08, "NVRAM controller"},
        {0x80, "Generic mass storage controller"}
    },
    {
        {0x00, "Ethernet controller"},
        {0x01, "Token ring controller"},
        {0x02, "FDDI controller"},
        {0x03, "ATM controller"},
        {0x04, "ISDN controller"},
        {0x05, "WorldFip controller"},
        {0x06, "PICMG 2.14 controller"},
        {0x07, "Infiniband controller"},
        {0x08, "Fabric controller"},
        {0x80, "Generic network controller"}
    },
    {
        {0x00, "VGA compatible controller"},
        {0x01, "XGA controller"},
        {0x02, "3D controller"},
        {0x80, "Generic display controller"}
    },
    {
        {0x00, "Video controller"},
        {0x01, "Audio controller"},
        {0x02, "Computer telephony device"},
        {0x03, "Audio device"},
        {0x80, "Generic multimedia controller"}
    },
    {
        {0x00, "RAM controller"},
        {0x01, "Flash controller"},
        {0x80, "Generic memory controller"}
    },
    {
        {0x00, "Host bridge"},
        {0x01, "ISA bridge"},
        {0x02, "EISA bridge"},
        {0x03, "MCA bridge"},
        {0x04, "PCI to PCI bridge"},
        {0x05, "PCMCIA bridge"},
        {0x06, "NuBus bridge"},
        {0x07, "CardBus bridge"},
        {0x08, "RACEway bridge"},
        {0x09, "PCI to PCI bridge"},
        {0x0A, "Infiniband to PCI host bridge"},
        {0x80, "Generic bridge"}
    },
    {
        {0x00, "Serial controller"},
        {0x01, "Parallel controller"},
        {0x02, "Multiport serial controller"},
        {0x03, "Modem"},
        {0x04, "IEEE 488.1/2 controller"},
        {0x05, "Smart Card controller"},
        {0x80, "Generic communication controller"}
    },
    {
        {0x00, "Programmable interrupt controller"},
        {0x01, "DMA controller"},
        {0x02, "System timer"},
        {0x03, "Real-time clock controller"},
        {0x04, "PCI Hotplug controller"},
        {0x05, "SD host controller"},
        {0x06, "IOMMU"},
        {0x80, "Generic base system peripheral"}
    },
    {
        {0x00, "Keyboard controller"},
        {0x01, "Digitizer pen"},
        {0x02, "Mouse controller"},
        {0x03, "Scanner controller"},
        {0x04, "Gameport controller"},
        {0x80, "Generic input device controller"}.
    },
    {
        {0x00, "Docking station"},
        {0x80, "Generic docking station"}
    },
    {
        {0x00, "Intel 386 processor"},
        {0x01, "Intel 486 processor"},
        {0x02, "Intel Pentium processor"},
        {0x03, "Intel Pentium Pro processor"},
        {0x10, "DEC Alpha processor"},
        {0x20, "IBM PowerPC processor"},
        {0x30, "MIPS processor"},
        {0x40, "Co-processor"},
        {0x80, "Generic processor"}
    },
    {
        {0x00, "FireWire controller"},
        {0x01, "ACCESS bus controller"},
        {0x02, "SSA controller"},
        {0x03, "USB controller"},
        {0x04, "Fibre channel"},
        {0x05, "SMBus controller"},
        {0x06, "InfiniBand controller"},
        {0x07, "IPMI interface"},
        {0x08, "SERCOS interface"},
        {0x09, "CANBus controller"},
        {0x80, "Generic serial bus controller"}
    },
    {
        {0x00, "iRDA compatible controller"},
        {0x01, "Consumer IR controller"},
        {0x10, "RF controller"},
        {0x11, "Bluetooth controller"},
        {0x12, "Broadband controller"},
        {0x20, "802.1a Ethernet controller"},
        {0x21, "802.1b Ethernet controller"},
        {0x80, "Generic wireless controller"}
    },
    {
        {0x0, "I20 controller"}
    },
    {
        {0x01, "Satellite TV controller"},
        {0x02, "Satellite audio controller"},
        {0x03, "Satellite voice controller"},
        {0x04, "Satellite data controller"}
    },
    {
        {0x00, "Network/Computing encryption controller"},
        {0x10, "Entertainment encryption controller"},
        {0x80, "Generic encryption controller"}
    },
    {
        {0x00, "DPIO module"},
        {0x01, "Performance counter"},
        {0x10, "Communication synchronizer"},
        {0x20, "Signal processing manager"},
        {0x80, "Generic signal processing controller"}
    }
};

void pci_enumerate();

#endif