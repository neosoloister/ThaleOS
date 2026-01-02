[org 0x7C00]
[BITS 16]

STAGE2_LBA     equ 1
STAGE2_SECTORS equ __STAGE2_SECTORS__
STAGE2_SEG     equ 0x0000
STAGE2_OFF     equ 0x8000

;==========================
;   MAIN
;==========================
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    cld

    mov [boot_drive], dl

    ; MBR
    mov si, mbr_msg
    call bios_print

    mov word [dap_sectors], STAGE2_SECTORS
    mov word [dap_buf_off], STAGE2_OFF
    mov word [dap_buf_seg], STAGE2_SEG
    mov dword [dap_lba_low], STAGE2_LBA
    mov dword [dap_lba_high], 0

    mov dl, [boot_drive]
    call disk_read_lba

    mov dl, [boot_drive]
    jmp STAGE2_SEG:STAGE2_OFF

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
;   Disk Address Packet
;==========================
dap:
    db 0x10, 0x00
dap_sectors:  dw 0
dap_buf_off:  dw 0
dap_buf_seg:  dw 0
dap_lba_low:  dd 0
dap_lba_high: dd 0

disk_read_lba:
    pusha
    mov si, dap
    mov ah, 0x42
    int 0x13
    jc hang
    popa
    ret

;==========================
;   DATA
;==========================
mbr_msg db "MBR OK!", 13, 10, 0
boot_drive db 0

;==========================
;   512 Boot Signature
;==========================
    times 510-($-$$) db 0
    db 0x55
    db 0xAA