#include <drivers/pci.h>
#include <kernel/kmalloc.h>
#include <kernel/klog.h>
#include <stddef.h>
#include <string.h>
#include <io.h>

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

const char* lookup(uint8_t class_code, pci_clookup_t* table, size_t len) {
    for(int i = 0; i < len; i++) {
        if(table[i].class_code == class_code) return table[i].name;
    }
    return "Unknown";
}

void pci_check_function(uint8_t bus, uint8_t dev, uint8_t func) {
    void* data = pci_get_data(bus, dev, func);
    pci_hc_t* hdr = (pci_hc_t*)data;
    size_t lut_len = 18;
    kprintf(LOG_INFO, "pci", "Device %02x.%02x:%x %s %04x:%04x\r\n",
        bus, dev, func,
        hdr->vid,
        hdr->devid,
        (hdr->class_code < lut_len)
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