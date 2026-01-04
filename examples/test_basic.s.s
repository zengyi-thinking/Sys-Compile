    .intel_syntax noprefix
    .text
    .global _start


_start:
    call main
    mov rdi, rax
    mov rax, 60
    syscall

main:
    push rbp
    mov rbp, rsp
    mov rax, 10
    mov rbx, 3.140000
    mov rcx, rax
    add rcx, rbx
    mov rdx, rbx
    neg rdx
    mov rax, rsi
    jmp .L_0_epilogue
    mov rsp, rbp
    pop rbp
    ret

