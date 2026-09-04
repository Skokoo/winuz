bits 16
section .mbr
global _m_start
_m_start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    jmp $

times 510-($-$$) db 0
dw 0xAA55

bits 32
section .text
global _start
extern go_64

_start:
    cli
    mov esp, st_t
    jmp go_64

section .bss
align 16
st_b: resb 16384
st_t: