#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "types.h"

void interrupts_initialize(void);

// Installs a task gate at IDT vector 'number' pointing at the given TSS
// selector. Used by tss_initialize() to wire up the double-fault task gate.
void interrupts_set_task_gate(int number, uint16_t selector);

// Shared helpers for printing a consistent "EXCEPTION: ..." report; also
// used by tss.c for the double-fault task-gate handler.
void exception_report(const char *name);
void exception_print_field(const char *label, uint32_t value, int hex);

void divide_error_handler(void);
void invalid_opcode_handler(void);
void general_protection_fault_handler(uint32_t error_code);
void page_fault_handler(uint32_t error_code);

void timer_handler(void);
// void keyboard_handler(void); in keyboard.h

#endif // INTERRUPTS_H