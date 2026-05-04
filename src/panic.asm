section .text
    global panic
panic:
    hlt
    jmp $ - 1