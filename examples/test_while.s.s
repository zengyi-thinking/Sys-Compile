    .intel_syntax noprefix
    .text
    .global _start


_start:
    call main
    mov rdi, rax
    mov rax, 60
    syscall

:
    push rbp
    mov rbp, rsp
L2:
    mov rsp, rbp
    pop rbp
    ret

