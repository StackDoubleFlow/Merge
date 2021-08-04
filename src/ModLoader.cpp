#include "ModLoader.h"
#include "Logger.h"
#include "MetadataBuilder.h"
#include "Mem.h"
#include "beatsaber-hook/shared/utils/il2cpp-functions.hpp"
#include "beatsaber-hook/shared/utils/utils.h"
#include "beatsaber-hook/shared/utils/instruction-parsing.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

MAKE_HOOK(MetadataCache_Register, nullptr, void, Il2CppCodeRegistration *codeRegistration, Il2CppMetadataRegistration *metadataRegistration, Il2CppCodeGenOptions *codeGenOptions) {
    MLogger::GetLogger().debug("MetadataCache_Register(%p, %p, %p)", codeRegistration, metadataRegistration, codeGenOptions);
    ModLoader::FixupCodeRegistration(codeRegistration, metadataRegistration, codeGenOptions);
    MetadataCache_Register(codeRegistration, metadataRegistration, codeGenOptions);
}

MAKE_HOOK(MetadataLoader_LoadMetadataFile, nullptr, void *, const char *fileName) {
    MLogger::GetLogger().debug("MetadataLoader_LoadMetadataFile(\"%s\")", fileName);
    return ModLoader::CreateNewMetadata();
}

std::vector<RawMod> ModLoader::rawMods;

void ModLoader::Initialize() {
    Logger &logger = MLogger::GetLogger();
    logger.info("Initializing ModLoader");

    il2cpp_functions::Init();
    Instruction ii = static_cast<const int32_t*>(HookTracker::GetOrig(il2cpp_functions::init));
    Instruction *j2R_I = CRASH_UNLESS(ii.findNthCall(2));
    Instruction Runtime_Init = CRASH_UNLESS(j2R_I->label);
    Instruction *j2MC_I = CRASH_UNLESS(Runtime_Init.findNthCall(9, 1));
    Instruction MetadataCache_Initialize = CRASH_UNLESS(j2MC_I->label);
    Instruction *j2ML_LMF = CRASH_UNLESS(MetadataCache_Initialize.findNthCall(1));
    int32_t *MetadataLoader_LoadMetadataFile = const_cast<int32_t *>(CRASH_UNLESS(j2ML_LMF->label));
    INSTALL_HOOK_DIRECT(logger, MetadataLoader_LoadMetadataFile, static_cast<void *>(MetadataLoader_LoadMetadataFile));

    Instruction *j2RRIAC_EI = CRASH_UNLESS(Runtime_Init.findNthCall(8, 1));
    int32_t *RegisterRuntimeInitializeAndCleanup_ExecuteInitializations = const_cast<int32_t *>(CRASH_UNLESS(j2RRIAC_EI->label));
    int64_t registrationCallbacksAddr = ExtractAddress(RegisterRuntimeInitializeAndCleanup_ExecuteInitializations, 1, 1);
    std::set<void (*)()> *registrationCallbacks = *reinterpret_cast<std::set<void (*)()> **>(registrationCallbacksAddr);
    Instruction s_Il2CppCodegenRegistration = reinterpret_cast<int32_t *>(*registrationCallbacks->begin());
    Instruction *j2icr = CRASH_UNLESS(s_Il2CppCodegenRegistration.findNthDirectBranchWithoutLink(1));
    Instruction il2cpp_codegen_register = CRASH_UNLESS(j2icr->label);
    Instruction *j2MC_R = CRASH_UNLESS(il2cpp_codegen_register.findNthDirectBranchWithoutLink(1));
    int32_t *MetadataCache_Register = const_cast<int32_t *>(CRASH_UNLESS(j2MC_R->label));
    INSTALL_HOOK_DIRECT(logger, MetadataCache_Register, MetadataCache_Register);

    g_MetadataRegistration = *reinterpret_cast<Il2CppMetadataRegistration **>(ExtractAddress(s_Il2CppCodegenRegistration.addr, 1, 1));
    g_CodeRegistration = reinterpret_cast<Il2CppCodeRegistration *>(ExtractAddress(s_Il2CppCodegenRegistration.addr, 2, 1));
    s_Il2CppCodeGenOptions = reinterpret_cast<Il2CppCodeGenOptions *>(ExtractAddress(s_Il2CppCodegenRegistration.addr, 3, 1));

    rawMods = ModReader::ReadAllMods();
    void *baseMetadata = ModReader::ReadBaseMetadata();
    MLogger::GetLogger().debug("ModLoader::Initialize with baseMetadata at %p", baseMetadata);
    metadataBuilder = MetadataBuilder(baseMetadata);

    for (auto &rawMod : rawMods) {
        metadataBuilder.AppendMetadata(rawMod.metadata, rawMod.modInfo.assemblyName, rawMod.runtimeMetadataTypeOffset);
    }
}

void *ModLoader::CreateNewMetadata() {
    return metadataBuilder.Finish();
}

int ModLoader::GetNextTypeIndex() {
    return g_MetadataRegistration->typesCount + addedTypes.size();
}

void ModLoader::FixupCodeRegistration(Il2CppCodeRegistration *&codeRegistration, Il2CppMetadataRegistration *&metadataRegistration, Il2CppCodeGenOptions *&codeGenOptions) {
    // codeRegistration->codeGenModules
    std::vector<const Il2CppCodeGenModule *> codeGenModules;
    for (size_t i = 0; i < codeRegistration->codeGenModulesCount; i++) {
        const Il2CppCodeGenModule *module = codeRegistration->codeGenModules[i];
        codeGenModules.push_back(module);
    }

    // codeRegistration->invokerPointers
    std::vector<InvokerMethod> invokerMethods;
    for (size_t i = 0; i < codeRegistration->invokerPointersCount; i++) {
        InvokerMethod method = codeRegistration->invokerPointers[i];
        invokerMethods.push_back(method);
    }

    // metadataRegistration->types
    std::vector<const Il2CppType *> types;
    for (size_t i = 0; i < metadataRegistration->typesCount; i++) {
        const Il2CppType *type = metadataRegistration->types[i];
        types.push_back(type);
    } 

    for (auto &rawMod : rawMods) {
        Instruction ii = static_cast<const int32_t*>(dlsym(rawMod.codeHandle, "il2cpp_init"));
        Instruction *j2R_I = CRASH_UNLESS(ii.findNthCall(2));
        Instruction Runtime_Init = CRASH_UNLESS(j2R_I->label);
        Instruction *j2RRIAC_EI = CRASH_UNLESS(Runtime_Init.findNthCall(8, 1));
        int32_t *RegisterRuntimeInitializeAndCleanup_ExecuteInitializations = const_cast<int32_t *>(CRASH_UNLESS(j2RRIAC_EI->label));
        int64_t registrationCallbacksAddr = ExtractAddress(RegisterRuntimeInitializeAndCleanup_ExecuteInitializations, 1, 1);
        std::set<void (*)()> *registrationCallbacks = *reinterpret_cast<std::set<void (*)()> **>(registrationCallbacksAddr);
        int32_t *s_Il2CppCodegenRegistration = reinterpret_cast<int32_t *>(*registrationCallbacks->begin());

        auto *g_MetadataRegistration = *reinterpret_cast<Il2CppMetadataRegistration **>(ExtractAddress(s_Il2CppCodegenRegistration, 1, 1));
        auto *g_CodeRegistration = reinterpret_cast<Il2CppCodeRegistration *>(ExtractAddress(s_Il2CppCodegenRegistration, 2, 1));
        auto *s_Il2CppCodeGenOptions = reinterpret_cast<Il2CppCodeGenOptions *>(ExtractAddress(s_Il2CppCodegenRegistration, 3, 1));

        rawMod.runtimeMetadataTypeOffset = types.size();
        MLogger::GetLogger().debug("Adding %i types to runtime metadata type registration at offset %lu", g_MetadataRegistration->typesCount, types.size());
        for (size_t i = 0; i < g_MetadataRegistration->typesCount; i++) {
            const Il2CppType *type = g_MetadataRegistration->types[i];
            types.push_back(type);
        }
        
        std::string moduleName = rawMod.modInfo.assemblyName + ".dll";
        for (size_t i = 0; i < g_CodeRegistration->codeGenModulesCount; i++) {
            Il2CppCodeGenModule *module = const_cast<Il2CppCodeGenModule *>(g_CodeRegistration->codeGenModules[i]);
            if (moduleName != module->moduleName) {
                continue;
            }
            MLogger::GetLogger().debug("Adding codegen module: %s", moduleName.c_str());
            int32_t invokerOffset = invokerMethods.size();
            for (size_t i = 0; i < g_CodeRegistration->invokerPointersCount; i++) {
                InvokerMethod invoker = g_CodeRegistration->invokerPointers[i];
                invokerMethods.push_back(invoker);
            }
            int32_t *invokerIndices = const_cast<int32_t *>(module->invokerIndices);
            MemProtect(invokerIndices, module->methodPointerCount * sizeof(int32_t *), PROT_READ | PROT_WRITE);
            for (size_t j = 0; j < module->methodPointerCount; j++) {
                invokerIndices[j] += invokerOffset;
            }
            codeGenModules.push_back(module);
            break;
        }
    }

    const Il2CppCodeGenModule **newCodegenModules = new const Il2CppCodeGenModule*[codeGenModules.size()];
    MLogger::GetLogger().debug("New codegen modules:");
    for (size_t i = 0; i < codeGenModules.size(); i++) {
        MLogger::GetLogger().debug(" %p: %s", codeGenModules[i], codeGenModules[i]->moduleName);
        newCodegenModules[i] = codeGenModules[i];
    }
    codeRegistration->codeGenModulesCount = codeGenModules.size();
    codeRegistration->codeGenModules = newCodegenModules;

    InvokerMethod *newInvokerMethods = new InvokerMethod[invokerMethods.size()];
    for (size_t i = 0; i < invokerMethods.size(); i++) {
        newInvokerMethods[i] = invokerMethods[i];
    }
    codeRegistration->invokerPointersCount = invokerMethods.size();
    codeRegistration->invokerPointers = newInvokerMethods;
    
    const Il2CppType **newTypes = new const Il2CppType*[types.size()];
    for (size_t i = 0; i < types.size(); i++) {
        newTypes[i] = types[i];
    }
    metadataRegistration->typesCount = types.size();
    metadataRegistration->types = newTypes;
}