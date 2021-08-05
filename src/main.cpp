#include "Logger.h"
#include "ModLoader.h"

// temporary
#include "merge-api.h"
#include "beatsaber-hook/shared/utils/utils.h"

void Dispose(Il2CppObject *self, MethodInfo *idk) {
    MLogger::GetLogger().info("SJDFHDSJGHSJKHGF IT WAS CALLED");
}

extern "C" void setup(ModInfo &info) {
    info.id = "Merge";
    info.version = VERSION;
    MLogger::modInfo = info;

    MLogger::GetLogger().info("Initializing Merge");
    ModLoader::Initialize();

    Merge::API::Initialize();

    TypeDefinitionIndex voidIdx = CRASH_UNLESS(Merge::API::FindTypeDefinitionIndex("System", "Void"));
    TypeDefinitionIndex objectIdx = CRASH_UNLESS(Merge::API::FindTypeDefinitionIndex("System", "Object"));
    TypeDefinitionIndex idisposableIdx = CRASH_UNLESS(Merge::API::FindTypeDefinitionIndex("System", "IDisposable"));

    AssemblyIndex assembly = Merge::API::CreateAssembly("MergeTest");
    ImageIndex image = Merge::API::CreateImage(assembly, "MergeTest.dll");

    Merge::API::MergeTypeDefinition poggersDef;
    poggersDef.name = "Poggers";
    poggersDef.namespaze = "Gaming";
    poggersDef.parent = Merge::API::GetTypeDefinition(objectIdx).byvalTypeIndex;
    poggersDef.attrs = 0;
    poggersDef.typeEnum = IL2CPP_TYPE_CLASS;
    poggersDef.valueType = false;
    poggersDef.interfaces = {Merge::API::GetTypeDefinition(idisposableIdx).byvalTypeIndex};
    std::vector<Merge::API::MergeTypeDefinition> types = {poggersDef};
    TypeDefinitionIndex poggersIdx = Merge::API::CreateTypes(image, std::span(types));

    Merge::API::MergeMethodDefinition disposeMethod;
    disposeMethod.name = "Dispose";
    disposeMethod.returnType = Merge::API::GetTypeDefinition(voidIdx).byvalTypeIndex;
    disposeMethod.methodPointer = (void(*)()) Dispose;
    std::vector<Merge::API::MergeMethodDefinition> methods = {disposeMethod};
    MethodIndex poggersDispose = Merge::API::CreateMethods(image, poggersIdx, std::span(methods));

    MethodIndex vDispose = CRASH_UNLESS(Merge::API::FindMethodDefinitionIndex(idisposableIdx, "Dispose", 0));
    Merge::API::OverridesMap overridesMap{{poggersDispose, vDispose}};
    Merge::API::SetMethodOverrides(poggersIdx, overridesMap);
}

template<typename T>
static T MetadataOffset(const void* metadata, size_t sectionOffset, size_t itemIndex) {
    return reinterpret_cast<T>(reinterpret_cast<const uint8_t*>(metadata) + sectionOffset) + itemIndex;
}

#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/byref.hpp"
extern "C" void load() {
    // MLogger::GetLogger().info("Found moles class: %s in %s", il2cpp_utils::ClassStandardName(moles).c_str(), moles->image->name);
    // MLogger::GetLogger().info("moles->image->codeGenModule->moduleName: %s", moles->image->codeGenModule->moduleName);
    // MLogger::GetLogger().info("moles->image->codeGenModule->invokerIndices[1]: %i", moles->image->codeGenModule->invokerIndices[1]);
    // MLogger::GetLogger().info("s_GlobalMetadataPtr %p", il2cpp_functions::s_GlobalMetadataHeader);

    // const void *s_GlobalMetadata = il2cpp_functions::s_GlobalMetadata;
    // const Il2CppGlobalMetadataHeader *s_GlobalMetadataHeader = il2cpp_functions::s_GlobalMetadataHeader;

    // MethodIndex start = moles->typeDefinition->methodStart;
    // MethodIndex end = start + moles->method_count;
    // for (MethodIndex index = start; index < end; ++index) {
    //     auto *method = MetadataOffset<const Il2CppMethodDefinition *>(s_GlobalMetadata, s_GlobalMetadataHeader->methodsOffset, index);
    //     auto *name = MetadataOffset<const char *>(s_GlobalMetadata, s_GlobalMetadataHeader->stringOffset, method->nameIndex);
    //     uint32_t rid = method->token & 0x00FFFFFF;
    //     MLogger::GetLogger().info("%s RID is %i", name, rid);
    //     int32_t invokerIndex = moles->image->codeGenModule->invokerIndices[rid - 1];
    //     MLogger::GetLogger().info("invokerIndex %i", invokerIndex);
    // }

    // for (int i = 0; i < s_GlobalMetadataHeader->methodsCount / sizeof(Il2CppMethodDefinition); i++) {
    //     auto *method = MetadataOffset<const Il2CppMethodDefinition *>(s_GlobalMetadata, s_GlobalMetadataHeader->methodsOffset, i);
    //     auto *name = MetadataOffset<const char *>(s_GlobalMetadata, s_GlobalMetadataHeader->stringOffset, method->nameIndex);
    //     if (std::string("getMoleCount") == name) {
    //         MLogger::GetLogger().info("getMoleCount RID is %i", method->token & 0x00FFFFFF);
    //     }
    // }
    // Il2CppClass *moles = il2cpp_utils::GetClassFromName("", "Moles");
    // const MethodInfo *molesPerDrive = il2cpp_utils::FindMethod(moles, "molesPerDrive", csTypeOf(Array<int>*));
    // Array<int> *arr = il2cpp_utils::RunMethodThrow<Array<int> *, false>(moles, molesPerDrive, 10);
    // MLogger::GetLogger().info("Moles.molesPerDrive returned length %i", arr->Length());

    // int molesCount = il2cpp_utils::RunMethodThrow<int>(moles, getMoleCount);
    // int molesCount = CRASH_UNLESS(il2cpp_utils::RunMethod<int>("", "Moles", "getMoleCount"));
}