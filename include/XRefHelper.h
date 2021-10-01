#pragma once
#include "stdint.h"
#include "beatsaber-hook/shared/utils/capstone-utils.hpp"

class XRefHelper {
public:
    void Init(const uint32_t *addr) {
        this->addr = addr;
    }

    // Easy init for HookTracker::GetOrig
    void Init(const void *addr) {
        this->addr = static_cast<const uint32_t *>(addr);
    }

    template<uint32_t n = 1, uint32_t rets = 0>
    const uint32_t *b() {
        addr = cs::findNthB<n, true, rets>(addr);
        return addr;
    }

    template<uint32_t n = 1, uint32_t rets = 0>
    const uint32_t *bl() {
        addr = cs::findNthBl<n, true, rets>(addr);
        return addr;
    }

    template<uint32_t n = 1>
    void *pcRelData() {
        return std::get<2>(cs::getpcaddr<n, 1>(addr));
    }

    template<typename T, uint32_t n = 1>
    T *pcRelData() {
        return reinterpret_cast<T *>(pcRelData<n>());
    }

    uint32_t *hookable() {
        return const_cast<uint32_t *>(addr);
    }
private:
    const uint32_t *addr;
};
