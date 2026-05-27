section .rodata
    const_pi dq 3.141592653589793
    const_e dq 2.718281828459045
    const_0 dq 10.539700
    const_1 dq 0.000000
    const_2 dq -1.000000
    const_3 dq 1.000000
    const_4 dq 0.500000
section .text
    global f_1, f_2, f_3, d_1, d_2, d_3
f_1:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi
    fld qword[const_0]
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret
d_1:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi
    fld qword[const_1]
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret
f_2:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi
    fld qword[const_2]
    fld qword[ebp+8]
    fdivp st1, st0
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret
d_2:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi
    fld qword[const_3]
    fld qword[ebp+8]
    fld qword[ebp+8]
    fmulp st1, st0
    fdivp st1, st0
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret
f_3:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi
    fld qword[ebp+8]
    fld qword[const_4]
    fsubp st1, st0
    fld qword[ebp+8]
    fld qword[const_4]
    fsubp st1, st0
    fmulp st1, st0
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret
d_3:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi
    fld qword[ebp+8]
    fld qword[const_4]
    fsubp st1, st0
    fld qword[ebp+8]
    fld qword[const_4]
    fsubp st1, st0
    faddp st1, st0
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret
