[org 0x7C00]
[bits 16]

_start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax

    ;==========================
    ; Set VGA mode
    ;==========================
    mov ax, 0x0003 ; 80x25
    int 0x10
    ; Enable cursor
    mov ax, 0x0100
    mov cx, 0x0607
    int 0x10

    ; Set up stack
    mov sp, 0x7000

    ; Enable the A20 line
    call    a20wait
    mov     al,0xAD
    out     0x64,al

    call    a20wait
    mov     al,0xD0
    out     0x64,al

    call    a20wait2
    in      al,0x60
    push    eax

    call    a20wait
    mov     al,0xD1
    out     0x64,al

    call    a20wait
    pop     eax
    or      al,2
    out     0x60,al

    call    a20wait
    mov     al,0xAE
    out     0x64,al
    jmp done

a20wait:
    in      al,0x64
    test    al,2
    jnz     a20wait
    ret
a20wait2:
    in      al,0x64
    test    al,1
    jz      a20wait2
    ret
done:

    ;==========================
    ; Load kernel to 0x7E00 (end of bootloader)
    ;==========================
    mov ax, 0x07E0          ; regment (address / 16)
    mov es, ax
    mov bx, 0x0000          ; offset in segment (added to segment * 16)
    mov ax, 0x0200 | KSIZE*2; sectors to read, KSIZE is in KiB
    mov ch, 0               ; cylinder 0
    mov cl, 2               ; sector 2 (1-based, first sector of kernel)
    mov dh, 0               ; head 0
    ; BIOS sets hard disk number for us
read_loop:
    int 0x13
    jc disk_error
    cli
    ; now kernel is at 0x7E00-0xFDFF

    ;==========================
    ; Enter protected mode
    ;==========================
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp 0x08:protected_mode_start

disk_error:
    hlt

[bits 32]
protected_mode_start:
    ; Setup data segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Jump to kernel entry point at 0x7E00
    jmp 0x00007E00
    hlt

;==========================
; GDT
;==========================
gdt_start:
    ; Null descriptor
    dq 0x0
    ; Code segment
    dq 0x00CF9B000000FFFF
    ; Data segment
    dq 0x00CF92000000FFFF
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

;==========================
; Boot signature
;==========================
times 510-($-$$) db 0
db 0x55
db 0xAA