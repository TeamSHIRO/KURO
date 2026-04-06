section .data
    global transfer_image_handle
    transfer_image_handle dq 0
    global transfer_system_table
    transfer_system_table dq 0
    global transfer_exec_info
    transfer_exec_info dq 0
    global transfer_boot_id_addr
    transfer_boot_id_addr dq 0
    global transfer_stack_start
    transfer_stack_start dq 0
    global transfer_entry_point
    transfer_entry_point dq 0

section .text
    global transfer_control
transfer_control:
    mov rdi, [transfer_exec_info]
    mov rsi, [transfer_image_handle]
    mov rdx, [transfer_system_table]
    xor rcx, rcx
    mov r8, [transfer_boot_id_addr]
    mov rsp, [transfer_stack_start]
    jmp [transfer_entry_point]