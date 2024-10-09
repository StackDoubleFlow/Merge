#include "Logger.h"
#include "ModLoader.h"

#include "scotland2/shared/modloader.h"

static modloader::ModInfo modInfo{MOD_ID, VERSION, 0};

#ifdef MERGE_TEST
#include "Tests.h"
#warning Compiling Merge with tests enabled
#endif

#define MOD_EXPORT __attribute__((visibility("default")))
#define MOD_EXTERN_FUNC extern "C" MOD_EXPORT

MOD_EXTERN_FUNC void setup(CModInfo *info) {
    *info = modInfo.to_c();

    MLogger.info("Initializing Merge");
    ModLoader::Initialize();

#ifdef MERGE_TEST
    MLogger.debug("Setting up %lu tests", MergeTests::setupTests.size());
    MergeTests::SetupTests();
#endif
}

extern "C" void load() {
#ifdef MERGE_TEST
    MLogger.debug("Running %lu tests", MergeTests::loadTests.size());
    MergeTests::LoadTests();
#endif
}