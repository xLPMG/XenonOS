#include "tss.h"
#include "interrupts.h"
#include "constants.h"

// 32-bit hardware TSS layout (Intel SDM Vol 3A, 7.2.1).
struct tss_entry
{
    uint32_t prev_task_link;
    uint32_t esp0;
    uint16_t ss0, reserved1;
    uint32_t esp1;
    uint16_t ss1, reserved2;
    uint32_t esp2;
    uint16_t ss2, reserved3;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax, ecx, edx, ebx, esp, ebp, esi, edi;
    uint16_t es, reserved4;
    uint16_t cs, reserved5;
    uint16_t ss, reserved6;
    uint16_t ds, reserved7;
    uint16_t fs, reserved8;
    uint16_t gs, reserved9;
    uint16_t ldt_selector, reserved10;
    uint16_t trap;
    uint16_t io_map_base;
} __attribute__((packed));

// Defined in boot.asm; each entry is one raw 8-byte GDT descriptor.
extern uint64_t gdt[];

extern void double_fault_task_entry(void);

static struct tss_entry main_tss;
static struct tss_entry double_fault_tss;
static uint8_t double_fault_stack[4096] __attribute__((aligned(16)));

static void gdt_set_descriptor(int index, uint32_t base, uint32_t limit,
                                uint8_t access, uint8_t granularity)
{
    gdt[index] = (uint64_t)(limit & 0xFFFF) |
                 ((uint64_t)(base & 0xFFFFFF) << 16) |
                 ((uint64_t)access << 40) |
                 ((uint64_t)(limit >> 16 & 0xF) << 48) |
                 ((uint64_t)(granularity & 0xF) << 52) |
                 ((uint64_t)(base >> 24 & 0xFF) << 56);
}

void tss_initialize(void)
{
    uint32_t cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));

    // Main TSS only needs to exist as a place for the CPU to save the
    // interrupted task's state; its own field values are never used.
    for (uint32_t i = 0; i < sizeof(main_tss); i++)
        ((uint8_t *)&main_tss)[i] = 0;
    gdt_set_descriptor(3, (uint32_t)&main_tss, sizeof(main_tss) - 1, 0x89, 0x00);

    // Double-fault TSS is pre-loaded with a known-good stack/CR3/entry point,
    // so the hardware task switch on #DF gives us guaranteed-valid state even
    // if the faulting task's own stack is completely broken.
    for (uint32_t i = 0; i < sizeof(double_fault_tss); i++)
        ((uint8_t *)&double_fault_tss)[i] = 0;
    double_fault_tss.cr3 = cr3;
    double_fault_tss.eip = (uint32_t)double_fault_task_entry;
    double_fault_tss.esp = (uint32_t)(double_fault_stack + sizeof(double_fault_stack));
    double_fault_tss.eflags = 0x2; // bit 1 is reserved and must always be 1
    double_fault_tss.cs = GDT_CODE_SELECTOR;
    double_fault_tss.ss = GDT_DATA_SELECTOR;
    double_fault_tss.ds = GDT_DATA_SELECTOR;
    double_fault_tss.es = GDT_DATA_SELECTOR;
    double_fault_tss.fs = GDT_DATA_SELECTOR;
    double_fault_tss.gs = GDT_DATA_SELECTOR;
    gdt_set_descriptor(4, (uint32_t)&double_fault_tss, sizeof(double_fault_tss) - 1, 0x89, 0x00);

    __asm__ volatile("ltr %%ax" ::"a"(GDT_MAIN_TSS_SELECTOR));

    interrupts_set_task_gate(8, GDT_DF_TSS_SELECTOR);
}

// Called from double_fault_task_entry (tss.asm) once the hardware task
// switch has landed us on the known-good double-fault stack.
void double_fault_task_handler(uint32_t error_code)
{
    exception_report("Double Fault (#8, task switch)");
    exception_print_field("Error code: ", error_code, 0);
}
