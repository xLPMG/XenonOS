#include "interrupts.h"
#include "terminal.h"
#include "string.h"
#include "scheduler.h"
#include "io_helper.h"
#include "pit.h"

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

extern void divide_error_isr(void);
extern void invalid_opcode_isr(void);
extern void double_fault_isr(void);
extern void general_protection_fault_isr(void);
extern void page_fault_isr(void);

extern void timer_isr(void);
extern void keyboard_isr(void);

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
    // CPU exceptions:        0–31
    // Hardware IRQs:         0–15
    // IDT interrupt vectors: 0–255

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

    // Mask everything except:
    // bit 0 = 0 -> timer enabled
    // bit 1 = 0 -> keyboard enabled
    outb(0x21, 0xFC);

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

    idt_set_gate(0, (uint32_t)divide_error_isr);
    idt_set_gate(6, (uint32_t)invalid_opcode_isr);
    idt_set_gate(8, (uint32_t)double_fault_isr);
    idt_set_gate(13, (uint32_t)general_protection_fault_isr);
    idt_set_gate(14, (uint32_t)page_fault_isr);

    idt_set_gate(32, (uint32_t)timer_isr);    // IRQ0 = interrupt 32
    idt_set_gate(33, (uint32_t)keyboard_isr); // IRQ1 = interrupt 33

    idt_descriptor.limit = sizeof(idt) - 1;
    idt_descriptor.base = (uint32_t)&idt;

    __asm__ volatile("lidtl %0" : : "m"(idt_descriptor));

    pic_remap();

    // Finally enable hardware interrupts
    __asm__ volatile("sti");
}

// MARK: Exception Handlers

static void exception_halt(void)
{
    __asm__ volatile("cli");

    while (1)
        __asm__ volatile("hlt");
}

void divide_error_handler(void)
{
    terminal_write("\n\nEXCEPTION: Divide Error (#0)\n");
    exception_halt();
}

void invalid_opcode_handler(void)
{
    terminal_write("\n\nEXCEPTION: Invalid Opcode (#6)\n");
    exception_halt();
}

void double_fault_handler(uint32_t error_code)
{
    char number[16];

    terminal_write("\n\nEXCEPTION: Double Fault (#8)\n");

    terminal_write("Error code: ");
    itoa(error_code, number);
    terminal_write(number);

    terminal_write("\n");

    exception_halt();
}

void general_protection_fault_handler(uint32_t error_code)
{
    char number[16];

    terminal_write("\n\nEXCEPTION: General Protection Fault (#13)\n");
    terminal_write("Error code: ");

    itoa(error_code, number);
    terminal_write(number);

    terminal_write("\n");

    exception_halt();
}

void page_fault_handler(uint32_t error_code)
{
    char number[16];

    uint32_t fault_address;

    __asm__ volatile(
        "mov %%cr2, %0"
        : "=r"(fault_address));

    terminal_write("\n\nEXCEPTION: Page Fault (#14)\n");

    terminal_write("Address: ");
    itoa_hex(fault_address, number);
    terminal_write(number);

    terminal_write("\nError code: ");
    itoa(error_code, number);
    terminal_write(number);

    terminal_write("\n");

    exception_halt();
}

// MARK: IRQ Handlers

void timer_handler(void)
{
    pit_tick();
    scheduler_yield();
}