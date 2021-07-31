#pragma once
#include <sys/mman.h>

// https://github.com/Rprop/And64InlineHook/blob/master/And64InlineHook.cpp#L474
#define __ptr(p) reinterpret_cast<void *>(p)
#define __align_up(x, n) (((x) + ((n)-1)) & ~((n)-1))
#define __align_down(x, n) ((x) & -(n))
#define __uintval(p) reinterpret_cast<uintptr_t>(p)
#define __page_size 4096
#define __page_align(n) __align_up(static_cast<uintptr_t>(n), __page_size)
#define __ptr_align(x)                                                         \
    __ptr(__align_down(reinterpret_cast<uintptr_t>(x), __page_size))

#define MemProtect(p, n, prot)                                                 \
    ::mprotect(__ptr_align(p),                                                 \
               __page_align(__uintval(p) + n) != __page_align(__uintval(p))    \
                   ? __page_align(n) + __page_size                             \
                   : __page_align(n),                                          \
               prot)

void MemDump(void *base, size_t n) {
    MLogger::GetLogger().debug("Memory dump: %p, n: %zu", base, n);
    for (size_t i = 0; i < n; i++) {
        MLogger::GetLogger().debug("%08x", static_cast<int32_t *>(base)[i]);
    }
}