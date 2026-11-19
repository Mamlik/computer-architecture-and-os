#pragma once
#include <stdint.h>
struct SpinLock {
    int64_t locked;
};

inline int64_t AtomicXchg(int64_t* atomic, int64_t value) {
    asm volatile (
                  "xchgq %0, %1"
                  : "+r"(value), "+m"(*atomic)
                  :
                  : "memory"
                  );
    return value;
}


inline void SpinLock_Init(struct SpinLock* lock) {
    lock->locked = 0;
}

inline void SpinLock_Lock(struct SpinLock* lock) {
    while (AtomicXchg(&lock->locked, 1)) {
        //wt
    }
    asm volatile ("":::"memory");
}

inline void SpinLock_Unlock(struct SpinLock* lock) {
    asm volatile ("":::"memory");
    lock->locked = 0;
}
