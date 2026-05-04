section .text
    global panic
panic:
    cli
.loop:
    hlt
    jmp .loop