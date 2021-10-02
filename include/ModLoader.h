#pragma once
#include "CodeGenModuleBuilder.h"
#include "MetadataBuilder.h"
#include "ModReader.h"
#include "TokenGenerator.h"

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
    static const Il2CppType *GetType(TypeIndex idx);
    static int GetTypesCount();
    static int GetInvokersCount();
    static void *CreateNewMetadata();

private:
    static bool initialized;
    static std::vector<RawMod> rawMods;

public:
    static MetadataBuilder metadataBuilder;
    static std::vector<Il2CppType *> addedTypes;
    static std::vector<InvokerMethod> addedInvokers;
    static std::vector<CustomAttributesCacheGenerator> addedCACacheGenerators;
    static std::unordered_map<ImageIndex, CodeGenModuleBuilder>
        addedCodeGenModules;
    static std::unordered_map<ImageIndex, TokenGenerator> tokenGenerators;
    
    // Il2CppCodeRegistration.cpp
    static const Il2CppMetadataRegistration *g_MetadataRegistration;
    static const Il2CppCodeRegistration *g_CodeRegistration;
    static const Il2CppCodeGenOptions *s_Il2CppCodeGenOptions;

    static std::unordered_map<TypeDefinitionIndex, int32_t> sizeOffsets;
};