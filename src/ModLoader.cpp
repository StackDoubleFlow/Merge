#include "ModLoader.h"
#include "Logger.h"
#include "MetadataBuilder.h"
#include "ModReader.h"
#include "beatsaber-hook/shared/utils/il2cpp-functions.hpp"
#include "beatsaber-hook/shared/utils/utils.h"
#include "beatsaber-hook/shared/utils/instruction-parsing.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"


MAKE_HOOK(MetadataLoader_LoadMetadataFile, nullptr, void *, const char *fileName) {
    void *baseMetadata = MetadataLoader_LoadMetadataFile(fileName);
    MLogger::GetLogger().debug("MetadataLoader_LoadMetadataFile(\"%s\") = %p", fileName, baseMetadata);
    void *newMetadata = ModLoader::LoadAllMods(baseMetadata);
    return newMetadata;
}

// void **ModLoader::FindGlobalMetadata() {
//     // Instruction cafc = static_cast<const int32_t*>(HookTracker::GetOrig(il2cpp_functions::custom_attrs_from_class));
//     // Instruction *j2MC_GCAI = CRASH_UNLESS(cafc.findNthCall(1));
//     // Instruction MetadataCache_GetCustomAttributeIndex = CRASH_UNLESS(j2MC_GCAI->label);
// 
//     // 1.16.4 MetadataCache::GetCustomAttributeIndex
//     //  adrp x9,0x337b000
//     //  ldr x9,[x9, #0x120]=>DAT_0337b120
//     void **addr = reinterpret_cast<void **>(baseAddr("libil2cpp.so") + 0x0337b120);
//     return addr;
// }

void ModLoader::Initialize() {
    il2cpp_functions::Init();
    Instruction ii = static_cast<const int32_t*>(HookTracker::GetOrig(il2cpp_functions::init));
    Instruction *j2R_I = CRASH_UNLESS(ii.findNthCall(2));
    Instruction Runtime_Init = CRASH_UNLESS(j2R_I->label);
    Instruction *j2MC_I = CRASH_UNLESS(Runtime_Init.findNthCall(9, 1));
    Instruction MetadataCache_Initialize = CRASH_UNLESS(j2MC_I->label);
    Instruction *j2ML_LMF = CRASH_UNLESS(MetadataCache_Initialize.findNthCall(1));
    int32_t *MetadataLoader_LoadMetadataFile = const_cast<int32_t *>(CRASH_UNLESS(j2ML_LMF->label));
    Logger &logger = MLogger::GetLogger();
    INSTALL_HOOK_DIRECT(logger, MetadataLoader_LoadMetadataFile, static_cast<void *>(MetadataLoader_LoadMetadataFile));
}

void *ModLoader::LoadAllMods(void *baseMetadata) {
    MLogger::GetLogger().debug("LoadAllMods with baseMetadata at %p", baseMetadata);

    MetadataBuilder builder;
    builder.AppendMetadata(baseMetadata);
    // for (auto &rawMod : rawMods) {
    //     builder.AppendMetadata(rawMod.metadata);
    // }

    // (*globalMetadataPtr) = builder.Finish();
    return builder.Finish();
}