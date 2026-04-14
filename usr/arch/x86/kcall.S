.section .text
.global kcall
//void kcall(
//  size_t id,
//  uintptr_t a1, uintptr_t a2, uintptr_t a3, uintptr_t a4
//);
kcall:
    push %ebp
    mov %esp, %ebp

    mov 4(%ebp), %eax
    mov 8(%ebp), %ebx
    mov 12(%ebp), %ecx
    mov 16(%ebp), %edx
    mov 20(%ebp), %edi

    int $0x80
    ret