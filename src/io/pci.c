#include "pci.h"

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

uint32_t pci_config_read32(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset)
{
    uint32_t address = (1u << 31) |
                        ((uint32_t)bus << 16) |
                        ((uint32_t)device << 11) |
                        ((uint32_t)function << 8) |
                        (offset & 0xFC);

    __asm__ volatile("outl %0, %1" : : "a"(address), "Nd"((uint16_t)PCI_CONFIG_ADDRESS));

    uint32_t value;
    __asm__ volatile("inl %1, %0" : "=a"(value) : "Nd"((uint16_t)PCI_CONFIG_DATA));
    return value;
}

int pci_find_vga_device(uint8_t *bus, uint8_t *device, uint8_t *function)
{
    for (uint32_t b = 0; b < 256; b++)
    {
        for (uint32_t d = 0; d < 32; d++)
        {
            uint32_t id = pci_config_read32((uint8_t)b, (uint8_t)d, 0, 0x00);
            if ((id & 0xFFFF) == 0xFFFF)
                continue; // no device present

            uint32_t class_reg = pci_config_read32((uint8_t)b, (uint8_t)d, 0, 0x08);
            uint8_t class_code = (uint8_t)(class_reg >> 24);
            uint8_t subclass = (uint8_t)(class_reg >> 16);

            if (class_code == 0x03 && subclass == 0x00)
            {
                *bus = (uint8_t)b;
                *device = (uint8_t)d;
                *function = 0;
                return 1;
            }
        }
    }

    return 0;
}

uint32_t pci_get_bar_address(uint8_t bus, uint8_t device, uint8_t function, uint8_t bar_index)
{
    uint32_t bar = pci_config_read32(bus, device, function, (uint8_t)(0x10 + bar_index * 4));

    // bit0 = 0 -> memory space BAR; low 4 bits are flags, not part of the address
    return bar & 0xFFFFFFF0u;
}
