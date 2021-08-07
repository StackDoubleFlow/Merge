#include "XRefHelper.h"
#include "beatsaber-hook/shared/utils/instruction-parsing.hpp"

void XRefHelper::Init(int32_t *addr) { this->addr = addr; }

void XRefHelper::Init(const void *addr) {
    this->addr = static_cast<int32_t *>(const_cast<void *>(addr));
}

int32_t *XRefHelper::b(int n, int rets) {
    Instruction cur(addr);
    Instruction *branch = cur.findNthDirectBranchWithoutLink(n, rets);
    CRASH_UNLESS(branch);
    const int32_t *label = CRASH_UNLESS(branch->label);
    addr = const_cast<int32_t *>(label);
    return addr;
}

int32_t *XRefHelper::bl(int n, int rets) {
    Instruction cur(addr);
    Instruction *branch = cur.findNthCall(n, rets);
    CRASH_UNLESS(branch);
    const int32_t *label = CRASH_UNLESS(branch->label);
    addr = const_cast<int32_t *>(label);
    return addr;
}

void *XRefHelper::pcRelData(int n) {
    Instruction cur(addr);
    void *data = reinterpret_cast<void *>(ExtractAddress(addr, n, 1));
    return data;
}