[org 0x8000]
[BITS 16]

KERNEL_LOAD_ADDR equ 0x00100000
KERNEL_SEG       equ 0x1000
KERNEL_OFF       equ 0x0000

KERNEL_LBA     equ __KERNEL_LBA__
KERNEL_SECTORS equ __KERNEL_SECTORS__

;==========================
;   MAIN
;==========================
    mov [boot_drive], dl

    xor ax, ax
    mov ds, ax
    mov es, ax

    cli
    mov ss, ax
    mov sp, 0x7C00
    sti

    cld

    mov si, stage2_msg
    call bios_print

    ; A20
    mov ax, 0x2401
    int 0x15
    jnc a20_ok
    jmp a20_fail

;==========================
;   BIOS Print
;==========================
bios_print:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E    ; BIOS Video Service
    mov bh, 0       ; Display page 0
    int 0x10        ; BIOS Teletype
    jmp bios_print
.done:
    ret

;==========================
;   GDT
;==========================
gdt_start:
gdt_null: dq 0x0000000000000000
gdt_code: dq 0x00CF9A000000FFFF
gdt_data: dq 0x00CF92000000FFFF
gdt_end:
gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

;==========================
;   Disk (LBA) - Load kernel
;==========================

a20_fail:
    in   al, 0x92
    or   al, 00000010b
    out  0x92, al

a20_ok:
    mov si, a20_msg
    call bios_print
    call load_kernel
    mov si, kernel_msg
    call bios_print
    jmp enable_pm

load_kernel:
    pusha

    mov dl, [boot_drive]
    mov ah, 0x41
    mov bx, 0x55AA
    int 0x13
    jc  disk_fail
    cmp bx, 0xAA55
    jne disk_fail
    test cx, 1
    jz  disk_fail

    mov word  [dap], 0x0010
    mov word  [dap+2], KERNEL_SECTORS
    mov word  [dap+4], KERNEL_OFF
    mov word  [dap+6], KERNEL_SEG
    mov dword [dap+8], KERNEL_LBA
    mov dword [dap+12], 0

    mov si, dap
    mov dl, [boot_drive]
    mov ah, 0x42
    int 0x13
    jc disk_fail

    popa
    ret

disk_fail:
    mov si, disk_err_msg
    call bios_print
.hang:
    hlt
    jmp .hang

;==========================
;   32-bits Protected Mode
;==========================
enable_pm:
    cli
    lgdt [gdt_descriptor]

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp 0x08:pm_entry

[BITS 32]
pm_entry:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000
    mov esi, 0x00010000
    mov edi, 0x00100000
    mov ecx, (__KERNEL_SECTORS__ * 512) / 4
    rep movsd
    cld

    jmp 0x08:KERNEL_LOAD_ADDR

pm_hang:
    hlt
    jmp pm_hang

;==========================
;   DATA
;==========================
boot_drive db 0

dap:
    db 0x10, 0x00
    dw 0
    dw 0, 0
    dq 0

a20_msg db "A20 OK!", 13, 10, 0
stage2_msg db "STAGE2 OK!", 13, 10, 0
kernel_msg db "KERNEL OK!", 13, 10, 0
disk_err_msg db "DISK ERR!", 13, 10, 0