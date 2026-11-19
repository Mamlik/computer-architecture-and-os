#include <kern/sysgate.h>
#include <kern/syscall.h>

#define IA32_EFER  0xC0000080
#define IA32_LSTAR 0xC0000082
#define IA32_EFER_SCE 1UL

long read_msr(int msr);
void write_msr(int msr, long value);

void _syscall_enter(void);
void _start_user(void);

void sysgate() {
    long efer = read_msr(IA32_EFER);
    efer |= IA32_EFER_SCE;
    write_msr(IA32_EFER, efer);

    write_msr(IA32_LSTAR, (long)_syscall_enter);

    asm volatile(
        "mov %[entr], %%rcx\n\t"
        "pushfq\n\t"
        "pop %%r11\n\t"
        "sysretq\n\t"
        :
        : [entr] "r" (_start_user)
        : "rcx", "r11", "memory"
    );
}
