bits 32

section .multiboot
align 8
m2_start:
    dd 0xe85250d6
    dd 0
    dd m2_end - m2_start
    dd -(0xe85250d6 + 0 + (m2_end - m2_start))

align 8
    dw 0
    dw 0
    dd 8
m2_end:

section .text
global _start
extern kmain

_start:
    cli
    mov esp, stack_top

    mov edi, 0x9000
    xor eax, eax
    mov ecx, 4096
    rep stosd

    mov dword [0x9000], 0xA003
    mov dword [0xA000], 0xB003
    mov dword [0xB000], 0x0000 | 0x83

    mov eax, 0x9000
    mov cr3, eax

    mov eax, cr4
    or eax, 32
    mov cr4, eax

    mov ecx, 0xC0000080
    rdmsr
    or eax, 256
    wrmsr
    
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    lgdt [g64_desc]

    push 0x08
    push init_64bit
    retf

bits 64
init_64bit:
    xor eax, eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov rdi, 1686005835
    mov rsi, 26985

    call kmain

.halt:
    hlt
    jmp .halt

align 8
g64:
    dq 0
.code: equ $ - g64
    dq 0x00209A0000000000
.data: equ $ - g64
    dq 0x0000920000000000
g64_desc:
    dw $ - g64 - 1
    dq g64

section .bss
align 16
stack_bottom:
    resb 16384
stack_top: