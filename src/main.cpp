#include "Logger.h"
#include "ModLoader.h"

extern "C" void setup(ModInfo &info) {
    info.id = "Merge";
    info.version = VERSION;
    MLogger::modInfo = info;

    MLogger::GetLogger().info("Initializing Merge");
    ModLoader::Initialize();
}

extern "C" void load() {}