#include "Logger.h"
#include "ModLoader.h"

#ifdef MERGE_TEST
#include "Tests.h"
#warning Compiling Merge with tests enabled
#endif

extern "C" void setup(ModInfo &info) {
    info.id = "Merge";
    info.version = VERSION;
    MLogger::modInfo = info;

    MLogger::GetLogger().info("Initializing Merge");
    ModLoader::Initialize();

#ifdef MERGE_TEST
    MLogger::GetLogger().debug("Setting up %lu tests",
                               MergeTests::setupTests.size());
    MergeTests::SetupTests();
#endif
}

extern "C" void load() {
#ifdef MERGE_TEST
    MLogger::GetLogger().debug("Running %lu tests",
                               MergeTests::loadTests.size());
    MergeTests::LoadTests();
#endif
}