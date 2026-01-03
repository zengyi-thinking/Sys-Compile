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
    mov rax, rax
    jmp .L_result_epilogue
    mov rsp, rbp
    pop rbp
    ret

