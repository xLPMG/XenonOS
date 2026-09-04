bits 32

section .text

global double_fault_task_entry
extern double_fault_task_handler

; Landing point after the hardware task switch on #DF. No pusha/iretd here:
; the CPU already loaded a full, known-good register/stack state from the
; double-fault TSS, and this never returns.
double_fault_task_entry:
    mov eax, [esp] ; hardware pushed the (always-zero) #DF error code
    push eax
    call double_fault_task_handler

.hang:
    cli
    hlt
    jmp .hang
