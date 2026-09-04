bits 32
section .text
global go_64
extern kmain

section .bss
align 4096
p4: resb 4096
p3: resb 4096
p2: resb 4096

section .text
go_64:
    mov edi, p4
    mov eax, p3
    or eax, 3
    mov [edi], eax

    mov edi, p3
    mov eax, p2
    or eax, 3
    mov [edi], eax

    mov edi, p2
    mov eax, 131
    mov [edi], eax

    mov eax, p4
    mov cr3, eax

    mov eax, cr4
    or eax, 32
    mov cr4, eax

    mov ecx, 3221225600
    rdmsr
    or eax, 256
    wrmsr

    mov eax, cr0
    or eax, 2147483648
    mov cr0, eax

    lgdt [g64.p]
    jmp g64.c:i64

bits 64
i64:
    mov dx, 1016
    mov al, 75
    out dx, al
    
    xor eax, eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    call kmain
    hlt

section .rodata
align 8
g64:
    dq 0
.c: equ $ - g64
    dq 107303033503744
.p:
    dw 15
    dq g64