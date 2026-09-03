bits 32

section .text

global context_switch

; void context_switch(uint32_t *old_esp, uint32_t new_esp);
; Saves the caller's callee-saved registers on the current stack, stashes
; the resulting esp into *old_esp, then switches to new_esp and restores
; the registers that were saved there when it was last switched away from.
context_switch:
    push ebp    ; frame/base pointer (callee-saved)
    push edi    ; destination index, general purpose (callee-saved)
    push esi    ; source index, general purpose (callee-saved)
    push ebx    ; general purpose (callee-saved)

    mov eax, [esp + 20]   ; old_esp argument (after the 4 pushes above)
    mov [eax], esp        ; *old_esp = current esp

    mov esp, [esp + 24]   ; load new_esp argument -> switch stacks

    pop ebx     ; general purpose (callee-saved)
    pop esi     ; source index, general purpose (callee-saved)
    pop edi     ; destination index, general purpose (callee-saved)
    pop ebp     ; frame/base pointer (callee-saved)

    ret
