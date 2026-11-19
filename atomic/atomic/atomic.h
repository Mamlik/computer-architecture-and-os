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

inline int64_t AtomicXchg(int64_t* atomic, int64_t value) {
    asm volatile (
                  "xchgq %0, %1"
                  : "+r"(value), "+m"(*atomic)
                  :
                  : "memory"
                  );
    return value;
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
//why not commiting???????
>>>>>>> 59fbafc (ebal github)
