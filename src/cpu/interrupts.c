#include "interrupts.h"

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

struct idt_entry
{
    uint16_t offset_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t type_attr;
    uint16_t offset_high;
} __attribute__((packed));

struct idt_ptr
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

static struct idt_entry idt[256];
static struct idt_ptr idt_descriptor;

extern void keyboard_isr(void);

static void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static void idt_set_gate(int number, uint32_t handler)
{
    idt[number].offset_low = handler & 0xFFFF;
    idt[number].selector = 0x08;
    idt[number].zero = 0;
    idt[number].type_attr = 0x8E;
    idt[number].offset_high = (handler >> 16) & 0xFFFF;
}

static void pic_remap(void)
{
    // Tell the master and slave PICs to start initialization
    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    // Remap IRQs:
    // Master: IRQ0-7 -> interrupts 32-39
    // Slave:  IRQ8-15 -> interrupts 40-47
    outb(0x21, 0x20);
    outb(0xA1, 0x28);

    // Tell master that slave is connected to IRQ2
    outb(0x21, 0x04);
    outb(0xA1, 0x02);

    // 8086 mode
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    // Mask everything except IRQ1 (keyboard)
    // Master:
    // bit 1 = 0 -> keyboard enabled
    outb(0x21, 0xFD);

    // Disable all slave IRQs.
    outb(0xA1, 0xFF);
}

void interrupts_initialize(void)
{
    // Clear IDT
    for (int i = 0; i < 256; i++)
    {
        idt[i].offset_low = 0;
        idt[i].selector = 0;
        idt[i].zero = 0;
        idt[i].type_attr = 0;
        idt[i].offset_high = 0;
    }

    // IRQ1 = interrupt 33
    idt_set_gate(33, (uint32_t)keyboard_isr);

    idt_descriptor.limit = sizeof(idt) - 1;
    idt_descriptor.base = (uint32_t)&idt;

    __asm__ volatile("lidtl %0" : : "m"(idt_descriptor));

    pic_remap();

    // Finally enable hardware interrupts
    __asm__ volatile("sti");
}