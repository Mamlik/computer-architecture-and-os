#include "malloc.h"

void* malloc(size_t size) {
    if (size == 0) {
        return NULL;
    }
    void* ptr = mmap(NULL, size + sizeof(size_t), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED) {
        return NULL;
    }
    *(size_t*)ptr = size;
    return (void*)((char*)ptr + sizeof(size_t));
}

void free(void* ptr) {
    if (ptr == NULL) {
        return;
    }
    ptr = (void*)((char*)ptr - sizeof(size_t));
    size_t size = *(size_t*)ptr + sizeof(size_t);
    munmap(ptr, size);
}

void* realloc(void* ptr, size_t size) {
    if (ptr == NULL) {
        return malloc(size);
    }
    if (size == 0) {
        free(ptr);
        return NULL;
    }
    ptr = (void*)((char*)ptr - sizeof(size_t));
    size_t old_size = *(size_t*)ptr + sizeof(size_t);
    void* new_ptr = mremap(ptr, old_size, size + sizeof(size_t), MREMAP_MAYMOVE, NULL);
    if (new_ptr == MAP_FAILED) {
        return NULL;
    }
    *(size_t*)new_ptr = size;
    return (void*)((char*)new_ptr + sizeof(size_t));
}
