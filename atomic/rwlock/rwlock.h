#pragma once
#include <stdint.h>

inline void AtomicAdd(int64_t* atomic, int64_t value) {
    asm volatile (
        "lock addq %1, %0"
        : "+m"(*atomic)
        : "r"(value)
        : "memory", "cc"
    );
}

inline void AtomicSub(int64_t* atomic, int64_t value) {
    asm volatile (
        "lock subq %1, %0"
        : "+m"(*atomic)
        : "r"(value)
        : "memory", "cc"
    );
}

inline int64_t AtomicCas(int64_t* atomic, int64_t* expected, int64_t value) {
    unsigned char result;
    int64_t old = *expected;
    asm volatile (
        "lock cmpxchgq %3, %1\n\t"
        "setz %0"
        : "=q"(result), "+m"(*atomic), "+a"(old)
        : "r"(value)
        : "memory", "cc"
    );
    *expected = old;
    return result;
}

inline int64_t AtomicXchg(int64_t* atomic, int64_t value) {
    asm volatile (
        "xchgq %0, %1"
        : "+r"(value), "+m"(*atomic)
        :
        : "memory"
    );
    return value;
}

struct SpinLock {
    int64_t locked;
};

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
    asm volatile("" ::: "memory");
    lock->locked = 0;
}

struct RwLock {
    struct SpinLock guard;
    int64_t readers;
    int64_t writer;
    int64_t waiting_writers;
};

inline void RwLock_Init(struct RwLock* lock) {
    SpinLock_Init(&lock->guard);
    lock->readers = 0;
    lock->writer = 0;
    lock->waiting_writers = 0;
}

inline void RwLock_ReadLock(struct RwLock* lock) {
    while (1) {
        SpinLock_Lock(&lock->guard);

        if (!lock->writer && lock->waiting_writers == 0) {
            ++lock->readers;
            SpinLock_Unlock(&lock->guard);
            asm volatile("" ::: "memory");
            return;
        }

        SpinLock_Unlock(&lock->guard);
    }
}

inline void RwLock_ReadUnlock(struct RwLock* lock) {
    asm volatile("" ::: "memory");
    SpinLock_Lock(&lock->guard);
    --lock->readers;
    SpinLock_Unlock(&lock->guard);
}

inline void RwLock_WriteLock(struct RwLock* lock) {
    SpinLock_Lock(&lock->guard);
    ++lock->waiting_writers;
    SpinLock_Unlock(&lock->guard);

    while (1) {
        SpinLock_Lock(&lock->guard);

        if (!lock->writer && lock->readers == 0) {
            --lock->waiting_writers;
            lock->writer = 1;
            SpinLock_Unlock(&lock->guard);
            asm volatile("" ::: "memory");
            return;
        }

        SpinLock_Unlock(&lock->guard);
    }
}

inline void RwLock_WriteUnlock(struct RwLock* lock) {
    asm volatile("" ::: "memory");
    SpinLock_Lock(&lock->guard);
    lock->writer = 0;
    SpinLock_Unlock(&lock->guard);
}
