#include "ModLoader.h"
#include "Logger.h"
#include "MetadataBuilder.h"
#include "ModReader.h"
#include "XRefHelper.h"
#define IL2CPP_FUNC_VISIBILITY public
#include "beatsaber-hook/shared/utils/hooking.hpp"

template <typename T>
static T MetadataSection(const void *metadata, size_t sectionOffset) {
    return reinterpret_cast<T>(reinterpret_cast<const uint8_t *>(metadata) +
                               sectionOffset);
}

MAKE_HOOK(MetadataCache_Register, nullptr, void,
          Il2CppCodeRegistration *codeRegistration,
          Il2CppMetadataRegistration *metadataRegistration,
          Il2CppCodeGenOptions *codeGenOptions) {
    MLogger.debug("MetadataCache_Register(%p, %p, %p)",
                  fmt::ptr(codeRegistration), fmt::ptr(metadataRegistration),
                  fmt::ptr(codeGenOptions));
    ModLoader::FixupCodeRegistration(codeRegistration, metadataRegistration,
                                     codeGenOptions);
    MetadataCache_Register(codeRegistration, metadataRegistration,
                           codeGenOptions);
}

MAKE_HOOK(MetadataLoader_LoadMetadataFile, nullptr, void *,
          const char *fileName) {
    MLogger.debug("MetadataLoader_LoadMetadataFile(\"%s\")", fileName);
    return ModLoader::CreateNewMetadata();
}

MAKE_HOOK(SetupFieldsLocked, nullptr, void, Il2CppClass *klass, void *lock) {
    bool sizeInited = klass->size_inited;

    SetupFieldsLocked(klass, lock);

    const Il2CppTypeDefinition *typeDefinition =
        reinterpret_cast<const Il2CppTypeDefinition *>(
            klass->typeMetadataHandle);
    if (sizeInited && typeDefinition) {
        return;
    }
    il2cpp_functions::CheckS_GlobalMetadata();

    auto *metadata =
        static_cast<const char *>(il2cpp_functions::s_GlobalMetadata);
    size_t sectionOffset =
        il2cpp_functions::s_GlobalMetadataHeader->typeDefinitionsOffset;

    auto *typeDefinitions =
        MetadataSection<const Il2CppTypeDefinition *>(metadata, sectionOffset);

    ptrdiff_t diff = typeDefinition - typeDefinitions;
    auto idx = static_cast<TypeDefinitionIndex>(diff);

    auto itr = ModLoader::sizeOffsets.find(idx);
    if (itr != ModLoader::sizeOffsets.end()) {
        klass->actualSize += itr->second;
        klass->instance_size += itr->second;
    }
}

bool ModLoader::initialized;

MetadataBuilder ModLoader::metadataBuilder;
std::vector<Il2CppType *> ModLoader::addedTypes;
std::vector<InvokerMethod> ModLoader::addedInvokers;
std::unordered_map<ImageIndex, CodeGenModuleBuilder>
    ModLoader::addedCodeGenModules;
std::unordered_map<ImageIndex, TokenGenerator> ModLoader::tokenGenerators;

const Il2CppMetadataRegistration *ModLoader::g_MetadataRegistration;
const Il2CppCodeRegistration *ModLoader::g_CodeRegistration;
const Il2CppCodeGenOptions *ModLoader::s_Il2CppCodeGenOptions;

std::unordered_map<TypeDefinitionIndex, int32_t> ModLoader::sizeOffsets;

void ModLoader::Initialize() {
    if (initialized)
        return;
    auto &logger = MLogger;
    logger.info("Initializing ModLoader");

    il2cpp_functions::Init();
    XRefHelper xref;

    xref.Init(HookTracker::GetOrig(il2cpp_functions::il2cpp_init));
    // Runtime::Init
    const uint32_t *Runtime_Init = xref.bl<2>();
    // MetadataCache::Initialize
    xref.bl<9, 1>();
    // MetadataLoader::LoadMetadataFile
    xref.bl<1>();
    INSTALL_HOOK_DIRECT(logger, MetadataLoader_LoadMetadataFile,
                        xref.hookable());

    xref.Init(Runtime_Init);
    // RegisterRuntimeInitializeAndCleanup::ExecuteInitializations
    xref.bl<8, 1>();
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
    xref.b();
    INSTALL_HOOK_DIRECT(logger, MetadataCache_Register, xref.hookable());

    xref.Init(s_Il2CppCodegenRegistration);
    g_MetadataRegistration = *xref.pcRelData<Il2CppMetadataRegistration *, 1>();
    g_CodeRegistration = xref.pcRelData<Il2CppCodeRegistration, 2>();
    s_Il2CppCodeGenOptions = xref.pcRelData<Il2CppCodeGenOptions, 3>();

    void *baseMetadata = ModReader::ReadBaseMetadata();
    MLogger.debug("ModLoader::Initialize with baseMetadata at %p",
                  baseMetadata);
    metadataBuilder.Initialize(baseMetadata);

    xref.Init(HookTracker::GetOrig(il2cpp_functions::il2cpp_object_new));
    // Object::New
    xref.bl();
    // Object::NewAllocSpecific
    xref.b();
    // Class::Init
    xref.bl();
    // InitLocked
    xref.bl<2>();
    // SetupFieldsLocked
    xref.bl<9>();
    INSTALL_HOOK_DIRECT(logger, SetupFieldsLocked, xref.hookable());

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

    // metadataRegistration->types
    VECTORIFY(const Il2CppType *, types, metadataRegistration->types);
    types.insert(types.end(), addedTypes.begin(), addedTypes.end());

    MLogger.debug("Final modules:");
    for (const Il2CppCodeGenModule *module : codeGenModules) {
        MLogger.debug(" %p: %s", fmt::ptr(module), module->moduleName);
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

    const Il2CppType **newTypes = new const Il2CppType *[types.size()];
    std::copy(types.begin(), types.end(), newTypes);
    metadataRegistration->types = newTypes;
    metadataRegistration->typesCount = types.size();
}