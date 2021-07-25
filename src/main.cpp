#include "Logger.h"

extern "C" void setup(ModInfo &info) {
    info.id = "Merge";
    info.version = VERSION;
    MLogger::modInfo = info;
}

extern "C" void load() {
    MLogger::GetLogger().info("Loading Merge");

    
}