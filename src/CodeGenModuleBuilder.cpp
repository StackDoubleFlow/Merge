#include "CodeGenModuleBuilder.h"

CodeGenModuleBuilder::CodeGenModuleBuilder(const char *moduleName) {
    this->moduleName = moduleName;
}

void CodeGenModuleBuilder::AppendMethod(Il2CppMethodPointer methodPointer,
                                        int32_t invokerIdx) {
    methodPointers.push_back(methodPointer);
    invokerIndices.push_back(invokerIdx);
}

Il2CppCodeGenModule *CodeGenModuleBuilder::Finish() {
    Il2CppCodeGenModule *mod =
        new Il2CppCodeGenModule{moduleName,
                                (uint32_t)methodPointers.size(),
                                nullptr,
                                (uint32_t)adjusterThunks.size(),
                                nullptr,
                                nullptr,
                                (uint32_t)reversePInvokeWrapperIndices.size(),
                                nullptr,
                                (uint32_t)rgctxRanges.size(),
                                nullptr,
                                (uint32_t)rgctxs.size(),
                                nullptr,
                                nullptr};

    auto *modMethodPointers = new Il2CppMethodPointer[methodPointers.size()];
    auto *modAdjusterThunks = new Il2CppTokenAdjustorThunkPair[adjusterThunks.size()];
    auto *modInvokerIndices = new int32_t[methodPointers.size()];
    auto *modReversePInvokeWrapperIndices =
        new Il2CppTokenIndexMethodTuple[reversePInvokeWrapperIndices.size()];
    auto *modRgctxRanges = new Il2CppTokenRangePair[rgctxRanges.size()];
    auto *modRgctxs = new Il2CppRGCTXDefinition[rgctxs.size()];

    std::copy(methodPointers.begin(), methodPointers.end(), modMethodPointers);
    std::copy(adjusterThunks.begin(), adjusterThunks.end(), modAdjusterThunks);
    std::copy(invokerIndices.begin(), invokerIndices.end(), modInvokerIndices);
    std::copy(reversePInvokeWrapperIndices.begin(),
              reversePInvokeWrapperIndices.end(),
              modReversePInvokeWrapperIndices);
    std::copy(rgctxRanges.begin(), rgctxRanges.end(), modRgctxRanges);
    std::copy(rgctxs.begin(), rgctxs.end(), modRgctxs);

    mod->methodPointers = modMethodPointers;
    mod->adjustorThunks = modAdjusterThunks;
    mod->invokerIndices = modInvokerIndices;
    mod->reversePInvokeWrapperIndices = modReversePInvokeWrapperIndices;
    mod->rgctxRanges = modRgctxRanges;
    mod->rgctxs = modRgctxs;

    return mod;
}

int32_t CodeGenModuleBuilder::GetNextMethodRID() {
    return methodPointers.size() + 1;
}
