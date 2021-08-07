#include "ModLoader.h"
#include "Logger.h"
#include "Mem.h"
#include "MetadataBuilder.h"
#include "XRefHelper.h"
#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-functions.hpp"
#include "beatsaber-hook/shared/utils/utils.h"

MAKE_HOOK(MetadataCache_Register, nullptr, void,
          Il2CppCodeRegistration *codeRegistration,
          Il2CppMetadataRegistration *metadataRegistration,
          Il2CppCodeGenOptions *codeGenOptions) {
    MLogger::GetLogger().debug("MetadataCache_Register(%p, %p, %p)",
                               codeRegistration, metadataRegistration,
                               codeGenOptions);
    ModLoader::FixupCodeRegistration(codeRegistration, metadataRegistration,
                                     codeGenOptions);
    MetadataCache_Register(codeRegistration, metadataRegistration,
                           codeGenOptions);
}

MAKE_HOOK(MetadataLoader_LoadMetadataFile, nullptr, void *,
          const char *fileName) {
    MLogger::GetLogger().debug("MetadataLoader_LoadMetadataFile(\"%s\")",
                               fileName);
    return ModLoader::CreateNewMetadata();
}

std::vector<RawMod> ModLoader::rawMods;
bool ModLoader::initialized;

MetadataBuilder ModLoader::metadataBuilder;
std::vector<Il2CppType *> ModLoader::addedTypes;
std::vector<CustomAttributesCacheGenerator> ModLoader::addedCACacheGenerators;
std::vector<InvokerMethod> ModLoader::addedInvokers;
std::unordered_map<ImageIndex, CodeGenModuleBuilder>
    ModLoader::addedCodeGenModules;
std::unordered_map<ImageIndex, TokenGenerator> ModLoader::tokenGenerators;

const Il2CppMetadataRegistration *ModLoader::g_MetadataRegistration;
const Il2CppCodeRegistration *ModLoader::g_CodeRegistration;
const Il2CppCodeGenOptions *ModLoader::s_Il2CppCodeGenOptions;

void ModLoader::Initialize() {
    if (initialized)
        return;
    Logger &logger = MLogger::GetLogger();
    logger.info("Initializing ModLoader");

    il2cpp_functions::Init();
    XRefHelper xref;

    xref.Init(HookTracker::GetOrig(il2cpp_functions::init));
    // Runtime::Init
    int32_t *Runtime_Init = xref.bl(2);
    // MetadataCache::Initialize
    xref.bl(9, 1);
    // MetadataLoader::LoadMetadataFile
    int32_t *MetadataLoader_LoadMetadataFile = xref.bl(1);
    INSTALL_HOOK_DIRECT(logger, MetadataLoader_LoadMetadataFile,
                        static_cast<void *>(MetadataLoader_LoadMetadataFile));

    xref.Init(Runtime_Init);
    // RegisterRuntimeInitializeAndCleanup::ExecuteInitializations
    xref.bl(8, 1);
    // static std::set<void (*)()>* il2cpp::utils::_registrationCallbacks
    auto *registrationCallbacks = *xref.pcRelData<std::set<void (*)()> *>();
    // registrationCallbacks should have one item in it:
    // void s_Il2CppCodegenRegistration()
    void *s_Il2CppCodegenRegistration =
        reinterpret_cast<void *>(*registrationCallbacks->begin());

    xref.Init(s_Il2CppCodegenRegistration);
    // il2cpp_codegen_register
    xref.b();
    // MetadataCache::Register
    int32_t *MetadataCache_Register = xref.b();
    INSTALL_HOOK_DIRECT(logger, MetadataCache_Register, MetadataCache_Register);

    xref.Init(s_Il2CppCodegenRegistration);
    g_MetadataRegistration = *xref.pcRelData<Il2CppMetadataRegistration *>(1);
    g_CodeRegistration = xref.pcRelData<Il2CppCodeRegistration>(2);
    s_Il2CppCodeGenOptions = xref.pcRelData<Il2CppCodeGenOptions>(3);

    rawMods = ModReader::ReadAllMods();
    void *baseMetadata = ModReader::ReadBaseMetadata();
    MLogger::GetLogger().debug("ModLoader::Initialize with baseMetadata at %p",
                               baseMetadata);
    metadataBuilder.Initialize(baseMetadata);

    for (auto &rawMod : rawMods) {
        metadataBuilder.AppendMetadata(rawMod.metadata,
                                       rawMod.modInfo.assemblyName,
                                       rawMod.runtimeMetadataTypeOffset);
    }

    initialized = true;
}

void *ModLoader::CreateNewMetadata() { return metadataBuilder.Finish(); }

int ModLoader::GetTypesCount() {
    return g_MetadataRegistration->typesCount + addedTypes.size();
}

int ModLoader::GetInvokersCount() {
    return g_CodeRegistration->invokerPointersCount + addedInvokers.size();
}

const Il2CppType *ModLoader::GetType(TypeIndex idx) {
    if (idx < g_MetadataRegistration->typesCount) {
        return g_MetadataRegistration->types[idx];
    } else if (idx - g_MetadataRegistration->typesCount < addedTypes.size()) {
        return addedTypes[idx - g_MetadataRegistration->typesCount];
    } else {
        return nullptr;
    }
}

#define VECTORIFY(type, vecName, arr)                                          \
    std::vector<type> vecName(arr, arr + arr##Count);

void ModLoader::FixupCodeRegistration(
    Il2CppCodeRegistration *&codeRegistration,
    Il2CppMetadataRegistration *&metadataRegistration,
    Il2CppCodeGenOptions *&codeGenOptions) {

    // codeRegistration->codeGenModules
    VECTORIFY(const Il2CppCodeGenModule *, codeGenModules,
              codeRegistration->codeGenModules);
    for (auto &[image, builder] : addedCodeGenModules) {
        codeGenModules.push_back(builder.Finish());
    }

    // codeRegistration->invokerPointers
    VECTORIFY(InvokerMethod, invokerMethods, codeRegistration->invokerPointers);
    invokerMethods.insert(invokerMethods.end(), addedInvokers.begin(),
                          addedInvokers.end());

    // codeRegistration->customAttributeGenerators
    std::vector<CustomAttributesCacheGenerator> caGenerators(
        codeRegistration->customAttributeGenerators,
        codeRegistration->customAttributeGenerators +
            codeRegistration->customAttributeCount);
    caGenerators.insert(caGenerators.end(), addedCACacheGenerators.begin(),
                        addedCACacheGenerators.end());

    // metadataRegistration->types
    VECTORIFY(const Il2CppType *, types, metadataRegistration->types);
    types.insert(types.end(), addedTypes.begin(), addedTypes.end());

    XRefHelper xref;
    for (auto &rawMod : rawMods) {
        // run xref on each mod to find g_MetadataRegistration,
        // g_CodeRegistration, and s_Il2CppCodeGenOptions

        xref.Init(dlsym(rawMod.codeHandle, "il2cpp_init"));
        // Runtime::Init
        int32_t *Runtime_Init = xref.bl(2);
        // RegisterRuntimeInitializeAndCleanup::ExecuteInitializations
        xref.bl(8, 1);
        // static std::set<void (*)()>* il2cpp::utils::_registrationCallbacks
        auto *registrationCallbacks = *xref.pcRelData<std::set<void (*)()> *>();
        // registrationCallbacks should have one item in it:
        // void s_Il2CppCodegenRegistration()
        void *s_Il2CppCodegenRegistration =
            reinterpret_cast<void *>(*registrationCallbacks->begin());

        xref.Init(s_Il2CppCodegenRegistration);
        auto *g_MetadataRegistration =
            *xref.pcRelData<Il2CppMetadataRegistration *>(1);
        auto *g_CodeRegistration = xref.pcRelData<Il2CppCodeRegistration>(2);
        auto *s_Il2CppCodeGenOptions = xref.pcRelData<Il2CppCodeGenOptions>(3);

        rawMod.runtimeMetadataTypeOffset = types.size();
        MLogger::GetLogger().debug("Adding %i types to runtime metadata type "
                                   "registration at offset %lu",
                                   g_MetadataRegistration->typesCount,
                                   types.size());
        for (size_t i = 0; i < g_MetadataRegistration->typesCount; i++) {
            const Il2CppType *type = g_MetadataRegistration->types[i];
            types.push_back(type);
        }

        std::string moduleName = rawMod.modInfo.assemblyName + ".dll";
        for (size_t i = 0; i < g_CodeRegistration->codeGenModulesCount; i++) {
            auto *module = const_cast<Il2CppCodeGenModule *>(
                g_CodeRegistration->codeGenModules[i]);
            if (moduleName != module->moduleName) {
                continue;
            }
            MLogger::GetLogger().debug("Adding codegen module: %s",
                                       moduleName.c_str());
            
            // Add invokers from mod
            int32_t invokerOffset = invokerMethods.size();
            size_t invokersCount = g_CodeRegistration->invokerPointersCount;
            for (size_t i = 0; i < invokersCount; i++) {
                InvokerMethod invoker = g_CodeRegistration->invokerPointers[i];
                invokerMethods.push_back(invoker);
            }

            // Offset mod's invoker indices
            int32_t *invokerIndices =
                const_cast<int32_t *>(module->invokerIndices);
            MemProtect(invokerIndices,
                       module->methodPointerCount * sizeof(int32_t *),
                       PROT_READ | PROT_WRITE);
            for (size_t j = 0; j < module->methodPointerCount; j++) {
                invokerIndices[j] += invokerOffset;
            }

            codeGenModules.push_back(module);
            break;
        }
    }

    for (const Il2CppCodeGenModule *module : codeGenModules) {
        MLogger::GetLogger().debug(" %p: %s", module, module->moduleName);
    }
    const Il2CppCodeGenModule **newCodegenModules =
        new const Il2CppCodeGenModule *[codeGenModules.size()];
    std::copy(codeGenModules.begin(), codeGenModules.end(), newCodegenModules);
    codeRegistration->codeGenModulesCount = codeGenModules.size();
    codeRegistration->codeGenModules = newCodegenModules;

    InvokerMethod *newInvokerMethods = new InvokerMethod[invokerMethods.size()];
    std::copy(invokerMethods.begin(), invokerMethods.end(), newInvokerMethods);
    codeRegistration->invokerPointers = newInvokerMethods;
    codeRegistration->invokerPointersCount = invokerMethods.size();

    CustomAttributesCacheGenerator *newCAGenerators =
        new CustomAttributesCacheGenerator[caGenerators.size()];
    std::copy(caGenerators.begin(), caGenerators.end(), newCAGenerators);
    codeRegistration->customAttributeGenerators = newCAGenerators;
    codeRegistration->customAttributeCount = caGenerators.size();

    const Il2CppType **newTypes = new const Il2CppType *[types.size()];
    std::copy(types.begin(), types.end(), newTypes);
    metadataRegistration->types = newTypes;
    metadataRegistration->typesCount = types.size();
}