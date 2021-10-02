#ifdef MERGE_TEST
#include "Tests.h"
#include "Logger.h"
#include "merge-api.h"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

using namespace Merge::API;

TEST_SETUP(OffsetSize) {
    auto logger = MLogger::GetLogger().WithContext("TestOffsetSize");
    logger.debug("Setting up test");

    // Random class with no fields
    TypeDefinitionIndex idx = FindTypeDefinitionIndex("UnityEngine", "Caching");
    CRASH_UNLESS(idx >= 0);

    OffsetSize(idx, 4);
}

TEST_LOAD(OffsetSize) {
    auto logger = MLogger::GetLogger().WithContext("TestOffsetSize");
    logger.debug("Running test");

    Il2CppClass *caching = CRASH_UNLESS(il2cpp_utils::GetClassFromName("UnityEngine", "Caching"));
    il2cpp_functions::il2cpp_Class_Init(caching);
    MLogger::GetLogger().debug("Actual size %u", caching->actualSize);
    MLogger::GetLogger().debug("Added size %lu", caching->actualSize - sizeof(Il2CppObject));
    CRASH_UNLESS(caching->actualSize == sizeof(Il2CppObject) + 4);
}

#endif