section .text
    global transfer_control
transfer_control:
    ; Assuming that System V is used when calling this function
    ; So we don't have to deal with setting up the args
    mov rax, [rsp + 8]
    mov rsp, r9
    jmp rax