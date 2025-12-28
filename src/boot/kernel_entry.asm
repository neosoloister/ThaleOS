; src/kernel_entry.asm
[BITS 32]

GLOBAL _start
EXTERN kernel_main

SECTION .text
_start:
    cli

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov esp, stack_top
    mov ebp, esp
    cld

    call kernel_main
.hang:
    hlt
    jmp .hang


SECTION .bss
align 16
stack_bottom:
    resb 16384
stack_top: