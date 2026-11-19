#include "backtrace.h"

#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

char* AddrToName(void* addr) {
    int fd = open("/proc/self/exe", O_RDONLY);
    if (fd == -1) {
        perror("open");
        return NULL;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("fstat");
        close(fd);
        return NULL;
    }

    void *map = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return NULL;
    }

    static char res[256];

    Elf64_Ehdr* ehdr = (Elf64_Ehdr*)map;
    Elf64_Shdr* shdr = (Elf64_Shdr*)((char*)ehdr + ehdr->e_shoff);
    Elf64_Shdr* shstrtab = &shdr[ehdr->e_shstrndx];

    for (int i = 0; i < ehdr->e_shnum; ++i) {
        char* secname = (char*)map + shstrtab->sh_offset + shdr[i].sh_name;
        if (strcmp(secname, ".symtab") == 0) {
            Elf64_Sym* symtab = (Elf64_Sym*)((char*)map + shdr[i].sh_offset);
            size_t sz = shdr[i].sh_size / shdr[i].sh_entsize;

            Elf64_Shdr* strtab = &shdr[shdr[i].sh_link];
            char* strtab_data = (char*)map + strtab->sh_offset;
            
            for (size_t j = 0; j < sz; ++j) {
                if (ELF64_ST_TYPE(symtab[j].st_info) == STT_FUNC) {
                    uintptr_t a = (uintptr_t)addr;
                    uintptr_t l = (uintptr_t)symtab[j].st_value;
                    uintptr_t r = l + symtab[j].st_size;

                    if (l <= a && a < r) {
                        char* name = strtab_data + symtab[j].st_name;
                        snprintf(res, sizeof(res), "%s", name);

                        if (munmap(map, st.st_size) == -1) {
                            perror("munmap");
                        }
                        close(fd);
                        return res;
                    }
                }
            }
        }
    }

    if (munmap(map, st.st_size) == -1) {
        perror("munmap");
    }
    close(fd);
    return NULL;
}

int Backtrace(void* backtrace[], int limit) {
    void* x;
    asm volatile(
        ".intel_syntax noprefix\n"
        "mov %0, rbp\n"
        ".att_syntax prefix\n"
        : "=r"(x)
    );
    int ans = 0;
    for (int i = 0; i < limit; ++i) {
        if (x == NULL) {
            break;
        }

        void* ret = *((void**)((char*)x + 8));
        backtrace[i] = ret;
        ans += 1;

        char* name = AddrToName(ret);
        if (name != NULL && strcmp(name, "main") == 0) {
            break;
        }

        x = *((void**)x);
    }
    return ans;
}

void PrintBt() {
    void* backtrace[256];
    int len = Backtrace(backtrace, 256);
    for (int i = 0; i < len; ++i) {
        char* name = AddrToName(backtrace[i]);
        printf("%p %s\n", backtrace[i], name);
    }
}
