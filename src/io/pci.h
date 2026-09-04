#ifndef PCI_H
#define PCI_H

#include "types.h"

// Minimal PCI config space access - just enough to locate the VGA display
// controller and read its BAR0 (linear framebuffer physical address).

uint32_t pci_config_read32(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);

// Scans bus 0 for a VGA-compatible display controller (class 0x03, subclass
// 0x00). Returns 1 and fills bus/device/function on success, 0 if not found.
int pci_find_vga_device(uint8_t *bus, uint8_t *device, uint8_t *function);

// Reads BAR(bar_index) for the given device and returns its base physical
// address (memory-mapped BARs only; low flag bits are masked off).
uint32_t pci_get_bar_address(uint8_t bus, uint8_t device, uint8_t function, uint8_t bar_index);

#endif // PCI_H
