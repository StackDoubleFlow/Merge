#include "Logger.h"
#include "ModLoader.h"
#include "Tests.h"

extern "C" void setup(ModInfo &info) {
    info.id = "Merge";
    info.version = VERSION;
    MLogger::modInfo = info;

    MLogger::GetLogger().info("Initializing Merge");
    ModLoader::Initialize();

#ifdef TEST
    MergeTests::SetupTests();
#endif
}

extern "C" void load() {
#ifdef TEST
    MergeTests::LoadTests();
#endif
}