#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "types.h"

void interrupts_initialize(void);

void divide_error_handler(void);
void invalid_opcode_handler(void);
void double_fault_handler(uint32_t error_code);
void general_protection_fault_handler(uint32_t error_code);
void page_fault_handler(uint32_t error_code);

void timer_handler(void);
// void keyboard_handler(void); in keyboard.h

#endif // INTERRUPTS_H