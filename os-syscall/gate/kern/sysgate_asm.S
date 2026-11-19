        .intel_syntax noprefix
        .text

    .global read_msr
read_msr:
    mov ecx, edi
    rdmsr
    shl rdx, 32
    or  rax, rdx
    ret

    .global write_msr
write_msr:
    mov ecx, edi
    mov rax, rsi
    mov rdx, rax
    shr rdx, 32
    wrmsr
    ret

    .extern sys_work
    .extern sys_retire

    .global _syscall_enter
_syscall_enter:
    push rcx
    push r11

    cmp rdi, 1
    je .do_work
    cmp rdi, 2
    je .do_retire

    mov rax, -1
    jmp .return_to_user

.do_work:
    mov rdi, rsi
    call sys_work
    mov rax, 5
    jmp .return_to_user

.do_retire:
    call sys_retire
    jmp .return_to_user

.return_to_user:
    pop r11
    pop rcx
    sysretq

        .att_syntax
