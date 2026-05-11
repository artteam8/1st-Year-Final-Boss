extern scanf, printf

section .data
    const_1 dd 1.0
    const_2 dd 2.0
    const_pi dd 3.141592653589793
    const_e dd 2.718281828459045
    fmt_in db "%f", 0
    fmt_out db "f(x) = %f", 10, 0

section .bss
    x resd 1

section .text
global main
main:
    push ebp
    mov ebp, esp
    
    push x
    push fmt_in
    call scanf
    add esp, 8
    
    push dword [x]
    call f
    add esp, 4
    
    sub esp, 8
    fstp qword [esp]
    push fmt_out
    call printf
    add esp, 12
    
    mov eax, 0
    pop ebp
    ret
