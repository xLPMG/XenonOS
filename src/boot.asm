bits 32

section .multiboot
align 8
mb_header_start:
    dd 0xE85250D6                               ; magic (multiboot2)
    dd 0                                        ; architecture: 0 = i386 protected mode
    dd mb_header_end - mb_header_start           ; header length
    dd -(0xE85250D6 + 0 + (mb_header_end - mb_header_start)) ; checksum

    ; Framebuffer request tag - included by default (real hardware), left out
    ; of the QEMU build (make xenonos-qemu.iso). Requesting one makes GRUB
    ; switch video hardware into a broken graphics state on this QEMU/SeaBIOS
    ; /GRUB combo regardless of VGA device emulated. framebuffer.c already
    ; falls back to programming the hardware directly if no tag is supplied.
%ifndef QEMU_BUILD
    align 8
    dw 5
    dw 0
    dd 20
    dd 1024
    dd 768
    dd 32
%endif

    ; End tag
    align 8
    dw 0
    dw 0
    dd 8
mb_header_end:

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