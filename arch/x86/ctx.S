.global ctx_switch
// void ctx_switch(*old_sp, new_sp)
ctx_switch:
    pusha
    mov 36(%esp), %eax      // load old_sp pointer
    mov %esp, (%eax)        // save current esp through the pointer
    mov 40(%esp), %esp      // load new_sp as the new stack pointer
    popa
    ret