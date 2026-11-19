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

inline int64_t AtomicLoad(int64_t* atomic) {
    int64_t expected = 0;
    AtomicCas(atomic, &expected, 0);
    return expected;
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
struct SeqLock {
    int64_t seq;
    struct SpinLock writer_lock;
};

inline void SeqLock_Init(struct SeqLock* lock) {
    lock->seq = 0;
    SpinLock_Init(&lock->writer_lock);
}

inline int64_t SeqLock_ReadLock(struct SeqLock* lock) {
    int64_t s;
    do {
        s = AtomicLoad(&lock->seq);
    } while ((s % 2) != 0);
    asm volatile("" ::: "memory");
    return s;
}

inline int SeqLock_ReadUnlock(struct SeqLock* lock, int64_t s) {
    asm volatile("" ::: "memory");
    return AtomicLoad(&lock->seq) == s;
}

inline void SeqLock_WriteLock(struct SeqLock* lock) {
    SpinLock_Lock(&lock->writer_lock);
    AtomicAdd(&lock->seq, 1);
    asm volatile("" ::: "memory");
}

inline void SeqLock_WriteUnlock(struct SeqLock* lock) {
    asm volatile("" ::: "memory");
    AtomicAdd(&lock->seq, 1);
    SpinLock_Unlock(&lock->writer_lock);
}
