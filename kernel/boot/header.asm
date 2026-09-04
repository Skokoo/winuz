bits 32
section .m2
align 8
m2_s:
    dd 1686005835
    dd 26985
    dd m2_e - m2_s
    dd -(1686005835 + 26985 + (m2_e - m2_s))
    dw 0, 0
    dd 8
m2_e: