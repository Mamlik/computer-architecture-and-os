#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>

extern size_t PAGE_SIZE;
extern double* SQRTS;
extern const int MAX_SQRTS;

static void* loaded_page = NULL;

void CalculateSqrts(double* sqrt_pos, int start, int n);

void HandleSigsegv(int sig, siginfo_t* siginfo, void* ctx) {
    char* fault = (char*)siginfo->si_addr;
    char* begin = (char*)SQRTS;
    char* end = begin + (size_t)MAX_SQRTS * sizeof(double);

    if (fault < begin || fault >= end) {
        _exit(1);
    }

    uintptr_t page_addr_num = ((uintptr_t)fault / PAGE_SIZE) * PAGE_SIZE;
    void* page_addr = (void*)page_addr_num;

    if (loaded_page != NULL && loaded_page != page_addr) {
        munmap(loaded_page, PAGE_SIZE);
    }

    if (loaded_page != page_addr) {
        void* p = mmap(page_addr, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);

        if (p == MAP_FAILED) {
            _exit(1);
        }

        int start = (int)(((char*)page_addr - begin) / sizeof(double));
        int n = (int)(PAGE_SIZE / sizeof(double));

        if (start + n > MAX_SQRTS) {
            n = MAX_SQRTS - start;
        }

        CalculateSqrts((double*)page_addr, start, n);
        loaded_page = page_addr;
    }
}
