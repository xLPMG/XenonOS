bits 32

section .text

global keyboard_isr
extern keyboard_handler

keyboard_isr:
    pusha

    ; Handle keyboard
    call keyboard_handler

    ; Tell the master PIC that IRQ1 is handled
    mov al, 0x20
    out 0x20, al

    popa
    iretd