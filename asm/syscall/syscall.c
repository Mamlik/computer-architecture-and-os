#include "syscall.h"

typedef long ssize_t;
typedef unsigned long size_t;

typedef int pid_t;

int open(const char* pathname, int flags) {
    int res;
    asm volatile ("syscall"
                  : "=a"(res)
                  : "a"(2), "D"(pathname), "S"(flags)
                  : "rcx", "r11", "memory");
    return res;
}

int close(int fd) {
    int res;
    asm volatile ("syscall"
                  : "=a"(res)
                  : "a"(3), "D"(fd)
                  : "rcx", "r11", "memory");
    return res;
}

ssize_t write(int fd, const void* buf, ssize_t count) {
    ssize_t res;
    asm volatile ("syscall"
                  : "=a"(res)
                  : "a"(1), "D"(fd), "S"(buf), "d"(count)
                  : "rcx", "r11", "memory");
    return res;
}

ssize_t read(int fd, void* buf, size_t count) {
    ssize_t res;
    asm volatile ("syscall"
                  : "=a"(res)
                  : "a"(0), "D"(fd), "S"(buf), "d"(count)
                  : "rcx", "r11", "memory");
    return res;
}

int pipe(int pipefd[2]) {
    int res;
    asm volatile ("syscall"
                  : "=a"(res)
                  : "a"(22), "D"(pipefd)
                  : "rcx", "r11", "memory");
    return res;
}

int dup(int oldfd) {
    int res;
    asm volatile ("syscall"
                  : "=a"(res)
                  : "a"(32), "D"(oldfd)
                  : "rcx", "r11", "memory");
    return res;
}

pid_t fork() {
    long ret;
    asm volatile ("syscall"
                  : "=a"(ret)
                  : "a"(57)
                  : "rcx", "r11", "memory");
    return ret;
}

pid_t waitpid(pid_t pid, int* wstatus, int options) {
    long ret;
    register long r10 asm("r10") = 0;

    asm volatile ("syscall"
                  : "=a"(ret)
                  : "a"(61), "D"(pid), "S"(wstatus), "d"(options), "r"(r10)
                  : "rcx", "r11", "memory");
    return (pid_t)ret;
}

int execve(const char* filename, char* const argv[], char* const envp[]) {
    long ret;
    asm volatile ("syscall"
                  : "=a"(ret)
                  : "a"(59), "D"(filename), "S"(argv), "d"(envp)
                  : "rcx", "r11", "memory");
    return ret;
}

__attribute__((noreturn))
void exit(int status) {
    asm volatile ("syscall"
                  :
                  : "a"(60), "D"(status)
                  : "rcx", "r11", "memory");
    __builtin_unreachable();
}
