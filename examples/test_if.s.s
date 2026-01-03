    .intel_syntax noprefix
    .text
    .global _start


_start:
    call main
    mov rdi, rax
    mov rax, 60
    syscall

int:
    push rbp
    mov rbp, rsp
    test rax, rax
    jz 
    mov rsp, rbp
    pop rbp
    ret

