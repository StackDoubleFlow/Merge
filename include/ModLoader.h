#pragma once
#include "MetadataBuilder.h"
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
    static int GetNextTypeIndex();
    static void *CreateNewMetadata();

private:
    static std::vector<RawMod> rawMods;

public:
    static MetadataBuilder metadataBuilder;
    static std::vector<Il2CppType> addedTypes;

    // Il2CppCodeRegistration.cpp
    static const Il2CppMetadataRegistration *g_MetadataRegistration;
    static const Il2CppCodeRegistration *g_CodeRegistration;
    static const Il2CppCodeGenOptions *s_Il2CppCodeGenOptions;
};