
#pragma once
#include "il2cpp-class-internals.h"
#include <string_view>
#include <unordered_map>
#include <vector>

class CodeGenModuleBuilder {
public:
    CodeGenModuleBuilder(const char *moduleName);
    void AppendMethod(Il2CppMethodPointer methodPointer, int32_t invokerIdx);
    Il2CppCodeGenModule *Finish();

private:
    const char *moduleName;
    std::vector<Il2CppMethodPointer> methodPointers;
    std::vector<int32_t> invokerIndices;
    std::vector<Il2CppTokenIndexMethodTuple> reversePInvokeWrapperIndices;
    std::vector<Il2CppTokenRangePair> rgctxRanges;
    std::vector<Il2CppRGCTXDefinition> rgctxs;
};