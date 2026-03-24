.global ctx_switch
// void ctx_switch(*old_sp, new_sp)
ctx_switch:
// Fake an IRET frame
//      EFLAGS
//      CS
// ESP->EIP
    mov (%esp), %eax // get RET addr
    pushf
    push %cs
    push %eax
    pusha
    mov 48(%esp), %eax      // load old_sp pointer
    mov %esp, (%eax)        // save current esp through the pointer
    mov 52(%esp), %esp      // load new_sp as the new stack pointer
    popa
    iret