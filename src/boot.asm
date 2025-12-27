; boot.asm
[org 0x7C00]
[BITS 16]

;==========================
;   MAIN
;==========================
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    cld

    ; MBR
    mov si, mbr_msg
    call bios_print
    
    ; A20
    mov ax, 0x2401
    int 0x15
    cli

    mov si, a20_msg
    call bios_print
    
    jmp enable_pm

hang:
    hlt
    jmp hang

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
    mov dword [0xB8000], 0x1F4B1F4F   ; 'O''K' with attributes

pm_hang:
    hlt
    jmp pm_hang

;==========================
;   DATA
;==========================
mbr_msg db "MBR OK!", 13, 10, 0
a20_msg db "A20 OK!", 13, 10, 0

;==========================
;   512 Boot Signature
;==========================
    times 510-($-$$) db 0
    db 0x55
    db 0xAA