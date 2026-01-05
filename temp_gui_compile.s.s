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
    mov rax, 0
    mov rbx, 0
    jmp L0
L1:
    test rcx, rcx
    jz L3
    mov rsi, rbx
    imul rsi, rdx
    mov rdi, rax
    add rdi, rsi
    mov rax, rdi
    jmp L4
L3:
    mov r8, rax
    add r8, rbx
    mov rax, r8
L4:
    mov r10, rbx
    add r10, r9
    mov rbx, r10
L0:
    test r11, r11
    jnz L1
L2:
    mov rax, rax
    jmp .L_sum_epilogue
    mov rsp, rbp
    pop rbp
    ret

