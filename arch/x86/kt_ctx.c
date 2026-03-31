#include <ctx.h>
#include <kernel/klog.h>

void kt_init_context(kt_context_t *ctx, void *arg, char priv) {
    uint32_t *stack_top = (uint32_t *)((uint8_t *)ctx->stack_base + THREAD_STACK_SIZE);
    uint32_t *sp = stack_top;

    uint32_t sel_data, sel_code;
    switch(priv) {
        case PRIV_KERNEL:
            sel_data = 0x10;
            sel_code = 0x08;
            break;
        default:
            kprintf(LOG_WARN, "kthread", "Invalid privilege level, defaulting to PRIV_USER\r\n");
        case PRIV_USER:
            sel_data = 0x23;
            sel_code = 0x1B;
            break;
    }

    *--sp = (uint32_t)arg;
    *--sp = (uint32_t)kthread_ret;

    // Fake an IRET frame
    // (p) means privilege switch
    // (p)  SS
    // (p)  ESP
    //      EFLAGS
    //      CS
    // ESP->EIP
    if(priv != PRIV_KERNEL) {
        *--sp = sel_data;
        *--sp = (uint32_t)stack_top;
    }
    *--sp = 0x0202;
    *--sp = sel_code;
    *--sp = (uint32_t)ctx->thread;

    *--sp = 0;  // eax
    *--sp = 0;  // ecx
    *--sp = 0;  // edx
    *--sp = 0;  // ebx
    *--sp = 0;  // esp (ignored)
    *--sp = 0;  // ebp
    *--sp = 0;  // esi
    *--sp = 0;  // edi

    // Segment registers
    *--sp = sel_data;
    *--sp = sel_data;
    *--sp = sel_data;
    *--sp = sel_data;

    ctx->sp = (uint32_t)sp;
}