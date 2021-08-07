#pragma once
#include "stdint.h"

class XRefHelper {
public:
    void Init(int32_t *addr);
    // Easy init for HookTracker::GetOrig
    void Init(const void *addr);
    int32_t *b(int n = 1, int rets = 0);
    int32_t *bl(int n = 1, int rets = 0);
    void *pcRelData(int n = 1);

    template<typename T>
    T *pcRelData(int n = 1) {
        return reinterpret_cast<T *>(pcRelData(n));
    }

private:
    int32_t *addr;
};