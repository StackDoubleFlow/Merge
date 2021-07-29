#pragma once
#include "ModReader.h"

struct Il2CppCodeRegistration;
struct Il2CppMetadataRegistration;
struct Il2CppCodeGenOptions;

class ModLoader {
public:
    static void Initialize();
    static void
    FixupCodeRegistration(Il2CppCodeRegistration *&codeRegistration,
                          Il2CppMetadataRegistration *&metadataRegistration,
                          Il2CppCodeGenOptions *&codeGenOptions);
    static void *CreateNewMetadata(void *baseMetadata);

private:
    static std::vector<RawMod> rawMods;
};