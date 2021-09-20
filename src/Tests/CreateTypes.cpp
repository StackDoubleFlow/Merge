#ifdef MERGE_TEST
#include "Tests.h"
#include "Logger.h"
#include "merge-api.h"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

using namespace Merge::API;

TEST_SETUP(CreateTypes) {
    auto logger = MLogger::GetLogger().WithContext("TestCreateTypes");
    logger.debug("Setting up test");

    TypeDefinitionIndex voidIdx = CRASH_UNLESS(FindTypeDefinitionIndex("System", "Void"));
    TypeDefinitionIndex objectIdx = CRASH_UNLESS(FindTypeDefinitionIndex("System", "Object"));

    AssemblyIndex assembly = CreateAssembly("CreateTypesTest");
    ImageIndex image = CreateImage(assembly, "CreateTypesTest.dll");

    MergeTypeDefinition poggersDef;
    poggersDef.name = "Poggers";
    poggersDef.namespaze = "Gaming";
    poggersDef.parent = GetTypeDefinition(objectIdx).byvalTypeIndex;
    poggersDef.attrs = 0;
    poggersDef.typeEnum = IL2CPP_TYPE_CLASS;
    poggersDef.valueType = false;
    poggersDef.interfaces = {};
    std::vector<MergeTypeDefinition> types = {poggersDef};
    TypeDefinitionIndex poggersIdx = CreateTypes(image, std::span(types));
}

TEST_LOAD(CreateTypes) {
    auto logger = MLogger::GetLogger().WithContext("TestCreateTypes");
    logger.debug("Running test");

    Il2CppClass *poggers = CRASH_UNLESS(il2cpp_utils::GetClassFromName("Gaming", "Poggers"));
    il2cpp_utils::LogClass(logger, poggers);
}

#endif