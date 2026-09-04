bits 32

section .multiboot
align 4

    dd 0x1BADB002
    dd 0x00000003
    dd -(0x1BADB002 + 0x00000003)

section .text
global _start
extern kmain

_start:
    cli

    ; Load GDT
    lgdt [gdt_descriptor]

    ; Load data segment selector
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Reload code segment
    jmp 0x08:protected_mode

protected_mode:
    ; Set up stack
    mov esp, stack_top

    ; GRUB:
    ; EAX = Multiboot magic
    ; EBX = Multiboot info address
    push ebx
    push eax

    call kmain

.hang:
    hlt
    jmp .hang


section .bss
align 16

stack_bottom:
    resb 16384
stack_top:


section .data
align 8

; Null descriptor
; 32-bit flat code segment
; 32-bit flat data segment
; Main TSS and double-fault TSS descriptors are filled in at runtime by
; tss_initialize() once the base addresses of the C-defined TSS structs are known.
global gdt
gdt:
    dq 0x0000000000000000
    dq 0x00CF9A000000FFFF
    dq 0x00CF92000000FFFF
    dq 0x0000000000000000
    dq 0x0000000000000000

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt - 1
    dd gdt