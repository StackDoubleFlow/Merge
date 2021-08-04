#pragma once
#include "MetadataBuilder.h"
#include "CodeGenModuleBuilder.h"
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
    static int GetTypesCount();
    static int GetInvokersCount();
    static void *CreateNewMetadata();

private:
    static std::vector<RawMod> rawMods;

public:
    static MetadataBuilder metadataBuilder;
    static std::vector<Il2CppType *> addedTypes;
    static std::vector<InvokerMethod> addedInvokers;
    static std::unordered_map<ImageIndex, CodeGenModuleBuilder>
        addedCodeGenModules;

    // Il2CppCodeRegistration.cpp
    static const Il2CppMetadataRegistration *g_MetadataRegistration;
    static const Il2CppCodeRegistration *g_CodeRegistration;
    static const Il2CppCodeGenOptions *s_Il2CppCodeGenOptions;
};