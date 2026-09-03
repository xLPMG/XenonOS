bits 32

section .text

global keyboard_isr
global divide_error_isr
global invalid_opcode_isr
global double_fault_isr
global general_protection_fault_isr
global page_fault_isr

extern keyboard_handler
extern divide_error_handler
extern invalid_opcode_handler
extern double_fault_handler
extern general_protection_fault_handler
extern page_fault_handler

; iretd = return from interrupt

; pusha/popa push and pop all 8 general-purpose registers in this fixed
; order (top of stack after pusha is edi, i.e. edi is pushed last):
;   eax - return value / scratch          (caller-saved)
;   ecx - scratch / loop counter          (caller-saved)
;   edx - scratch                        (caller-saved)
;   ebx - general purpose                (callee-saved)
;   esp - original stack pointer (ignored by popa, esp isn't restored from it)
;   ebp - frame/base pointer              (callee-saved)
;   esi - source index, general purpose   (callee-saved)
;   edi - destination index, general purpose (callee-saved)

keyboard_isr:
    pusha
    call keyboard_handler

    mov al, 0x20
    out 0x20, al

    popa
    iretd


divide_error_isr:
    pusha
    call divide_error_handler
    popa
    iretd


invalid_opcode_isr:
    pusha
    call invalid_opcode_handler
    popa
    iretd


double_fault_isr:
    pusha

    mov eax, [esp + 32]
    push eax

    call double_fault_handler

    add esp, 4
    popa
    iretd


general_protection_fault_isr:
    pusha

    ; CPU pushed the error code before entering the ISR.
    ; pusha pushed another 32 bytes.
    mov eax, [esp + 32]
    push eax

    call general_protection_fault_handler

    add esp, 4
    popa
    iretd


page_fault_isr:
    pusha

    ; CPU pushed the page-fault error code.
    mov eax, [esp + 32]
    push eax

    call page_fault_handler

    add esp, 4
    popa
    iretd