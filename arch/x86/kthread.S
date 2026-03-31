.global _isr32
_isr32:
    pusha
    push %gs
    push %fs
    push %es
    push %ds

    push $32
    call pic_eoi
    add $4, %esp

    push $n_sp
    push $c_sp
    call kthread_schedule
    add $8, %esp

    // Get the results
    mov n_sp, %eax
    test %eax, %eax
    jz .zero // No context change needed

    mov c_sp, %ebx
    mov %esp, (%ebx) // Save curr_sp

    mov (%eax), %esp // Set ESP to new_sp

.zero:
    pop %ds
    pop %es
    pop %fs
    pop %gs
    popa
    iret

.data
c_sp:   .long 0
n_sp:   .long 0
