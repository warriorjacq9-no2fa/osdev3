#include <drivers/pci.h>
#include <kernel/kmalloc.h>
#include <kernel/klog.h>
#include <stddef.h>
#include <string.h>
#include <io.h>

typedef struct pci_class_lookup {
    uint8_t class_code;
    const char* name;
} pci_clookup_t;

/* =========================
 * PCI subclass tables
 * ========================= */

static const pci_clookup_t pci_class_0[] = {
    {0x00, "Unclassified device"},
    {0x01, "VGA compatible"}
};

static const pci_clookup_t pci_class_1[] = {
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
};

static const pci_clookup_t pci_class_2[] = {
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
};

static const pci_clookup_t pci_class_3[] = {
    {0x00, "VGA compatible controller"},
    {0x01, "XGA controller"},
    {0x02, "3D controller"},
    {0x80, "Generic display controller"}
};

static const pci_clookup_t pci_class_4[] = {
    {0x00, "Video controller"},
    {0x01, "Audio controller"},
    {0x02, "Computer telephony device"},
    {0x03, "Audio device"},
    {0x80, "Generic multimedia controller"}
};

static const pci_clookup_t pci_class_5[] = {
    {0x00, "RAM controller"},
    {0x01, "Flash controller"},
    {0x80, "Generic memory controller"}
};

static const pci_clookup_t pci_class_6[] = {
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
};

static const pci_clookup_t pci_class_7[] = {
    {0x00, "Serial controller"},
    {0x01, "Parallel controller"},
    {0x02, "Multiport serial controller"},
    {0x03, "Modem"},
    {0x04, "IEEE 488.1/2 controller"},
    {0x05, "Smart Card controller"},
    {0x80, "Generic communication controller"}
};

static const pci_clookup_t pci_class_8[] = {
    {0x00, "Programmable interrupt controller"},
    {0x01, "DMA controller"},
    {0x02, "System timer"},
    {0x03, "Real-time clock controller"},
    {0x04, "PCI Hotplug controller"},
    {0x05, "SD host controller"},
    {0x06, "IOMMU"},
    {0x80, "Generic base system peripheral"}
};

static const pci_clookup_t pci_class_9[] = {
    {0x00, "Keyboard controller"},
    {0x01, "Digitizer pen"},
    {0x02, "Mouse controller"},
    {0x03, "Scanner controller"},
    {0x04, "Gameport controller"},
    {0x80, "Generic input device controller"}
};

static const pci_clookup_t pci_class_10[] = {
    {0x00, "Docking station"},
    {0x80, "Generic docking station"}
};

static const pci_clookup_t pci_class_11[] = {
    {0x00, "Intel 386 processor"},
    {0x01, "Intel 486 processor"},
    {0x02, "Intel Pentium processor"},
    {0x03, "Intel Pentium Pro processor"},
    {0x10, "DEC Alpha processor"},
    {0x20, "IBM PowerPC processor"},
    {0x30, "MIPS processor"},
    {0x40, "Co-processor"},
    {0x80, "Generic processor"}
};

static const pci_clookup_t pci_class_12[] = {
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
};

static const pci_clookup_t pci_class_13[] = {
    {0x00, "iRDA compatible controller"},
    {0x01, "Consumer IR controller"},
    {0x10, "RF controller"},
    {0x11, "Bluetooth controller"},
    {0x12, "Broadband controller"},
    {0x20, "802.1a Ethernet controller"},
    {0x21, "802.1b Ethernet controller"},
    {0x80, "Generic wireless controller"}
};

static const pci_clookup_t pci_class_14[] = {
    {0x00, "I20 controller"}
};

static const pci_clookup_t pci_class_15[] = {
    {0x01, "Satellite TV controller"},
    {0x02, "Satellite audio controller"},
    {0x03, "Satellite voice controller"},
    {0x04, "Satellite data controller"}
};

static const pci_clookup_t pci_class_16[] = {
    {0x00, "Network/Computing encryption controller"},
    {0x10, "Entertainment encryption controller"},
    {0x80, "Generic encryption controller"}
};

static const pci_clookup_t pci_class_17[] = {
    {0x00, "DPIO module"},
    {0x01, "Performance counter"},
    {0x10, "Communication synchronizer"},
    {0x20, "Signal processing manager"},
    {0x80, "Generic signal processing controller"}
};

/* =========================
 * Pointer table
 * ========================= */

static const pci_clookup_t* pci_classes[] = {
    pci_class_0,
    pci_class_1,
    pci_class_2,
    pci_class_3,
    pci_class_4,
    pci_class_5,
    pci_class_6,
    pci_class_7,
    pci_class_8,
    pci_class_9,
    pci_class_10,
    pci_class_11,
    pci_class_12,
    pci_class_13,
    pci_class_14,
    pci_class_15,
    pci_class_16,
    pci_class_17
};

/* =========================
 * Length table
 * ========================= */

static const size_t pci_class_lengths[] = {
    sizeof(pci_class_0)  / sizeof(pci_clookup_t),
    sizeof(pci_class_1)  / sizeof(pci_clookup_t),
    sizeof(pci_class_2)  / sizeof(pci_clookup_t),
    sizeof(pci_class_3)  / sizeof(pci_clookup_t),
    sizeof(pci_class_4)  / sizeof(pci_clookup_t),
    sizeof(pci_class_5)  / sizeof(pci_clookup_t),
    sizeof(pci_class_6)  / sizeof(pci_clookup_t),
    sizeof(pci_class_7)  / sizeof(pci_clookup_t),
    sizeof(pci_class_8)  / sizeof(pci_clookup_t),
    sizeof(pci_class_9)  / sizeof(pci_clookup_t),
    sizeof(pci_class_10) / sizeof(pci_clookup_t),
    sizeof(pci_class_11) / sizeof(pci_clookup_t),
    sizeof(pci_class_12) / sizeof(pci_clookup_t),
    sizeof(pci_class_13) / sizeof(pci_clookup_t),
    sizeof(pci_class_14) / sizeof(pci_clookup_t),
    sizeof(pci_class_15) / sizeof(pci_clookup_t),
    sizeof(pci_class_16) / sizeof(pci_clookup_t),
    sizeof(pci_class_17) / sizeof(pci_clookup_t)
};

uint32_t pci_read_word(uint8_t bus, uint8_t dev, uint8_t func, uint8_t off) {
    pci_cfg_addr_t address = {
        .cfg = {
            .en = 1,
            .bus = bus,
            .dev = dev,
            .func = func,
            .off = off & 0xFC
        }
    };

    outl(PCI_CONFIG_ADDR, address.val);
    return inl(PCI_CONFIG_DATA);
}

uint16_t pci_read_short(uint8_t bus, uint8_t dev, uint8_t func, uint8_t off) {
    uint32_t word = pci_read_word(bus, dev, func, off);
    size_t shift = off & 2;
    return (uint16_t)((word >> (shift * 8)) & 0xFFFF);
}

uint8_t pci_read_byte(uint8_t bus, uint8_t dev, uint8_t func, uint8_t off) {
    uint32_t word = pci_read_word(bus, dev, func, off);
    size_t shift = off & 3;
    return (uint8_t)((word >> (shift * 8)) & 0xFF);
}

uint16_t pci_get_vendor(uint8_t bus, uint8_t dev, uint8_t func) {
    return pci_read_short(bus, dev, func, offsetof(pci_hc_t, vid));
}

void* pci_get_data(uint8_t bus, uint8_t dev, uint8_t func) {
    if(pci_get_vendor(bus, dev, func) == 0xFFFF) return NULL;
    uint32_t data[sizeof(pci_hc_t) / sizeof(uint32_t)];
    for(size_t i = 0; i < sizeof(pci_hc_t); i += sizeof(uint32_t)) {
        data[i / sizeof(uint32_t)] = pci_read_word(bus, dev, func, i);
    }
    pci_hc_t* hc = (pci_hc_t*)data;
    void* ret = NULL;
    if((hc->header_type & 0x7F) == 0) {
        ret = kmalloc(sizeof(pci_hc_t) + sizeof(pci_h0_t), 0);
        memcpy(ret, hc, sizeof(pci_hc_t));
        uint32_t* buf = (uint32_t*)((uint8_t*)ret + sizeof(pci_hc_t));
        for(size_t i = 0; i < sizeof(pci_h0_t); i += sizeof(uint32_t)) {
            buf[i / sizeof(uint32_t)] = pci_read_word(bus, dev, func, sizeof(pci_hc_t) + i);
        }
    } else if((hc->header_type & 0x7F) == 1) {
        ret = kmalloc(sizeof(pci_hc_t) + sizeof(pci_h1_t), 0);
        memcpy(ret, hc, sizeof(pci_hc_t));
        uint32_t* buf = (uint32_t*)((uint8_t*)ret + sizeof(pci_hc_t));
        for(size_t i = 0; i < sizeof(pci_h1_t); i += sizeof(uint32_t)) {
            buf[i / sizeof(uint32_t)] = pci_read_word(bus, dev, func, sizeof(pci_hc_t) + i);
        }
    }
    return ret;
}

const char* pci_get_classname(uint8_t class_code, uint8_t subclass)
{
    if (class_code >= 18)
        return "Unknown class";

    for (size_t i = 0; i < pci_class_lengths[class_code]; i++) {
        if (pci_classes[class_code][i].class_code == subclass)
            return pci_classes[class_code][i].name;
    }

    return "Unknown subclass";
}

void pci_check_function(uint8_t bus, uint8_t dev, uint8_t func) {
    void* data = pci_get_data(bus, dev, func);
    pci_hc_t* hdr = (pci_hc_t*)data;
    kprintf(LOG_INFO, "pci", "Device %02x.%02x:%x %s %04x:%04x\r\n",
        bus, dev, func,
        pci_get_classname(hdr->class_code, hdr->subclass),
        hdr->vid,
        hdr->devid
    );
}

void pci_enumerate() {
    for(int bus = 0; bus < 256; bus++) {
        for(int dev = 0; dev < 32; dev++) {
            uint16_t vendor = pci_get_vendor(bus, dev, 0);
            if(vendor == 0xFFFF) continue;
            pci_check_function(bus, dev, 0);
            uint8_t htype = pci_read_byte(bus, dev, 0, offsetof(pci_hc_t, header_type));
            if(htype & 0x80) {
                for(int func = 1; func < 8; func++) {
                    vendor = pci_get_vendor(bus, dev, func);
                    if(vendor != 0xFFFF) {
                        pci_check_function(bus, dev, func);
                    }
                }
            }
        }
    }
}