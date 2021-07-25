#include "Logger.h"

extern "C" void setup(ModInfo &info) {
    info.id = "Merge";
    info.version = VERSION;
    CJDLogger::modInfo = info;
}

extern "C" void load() {
    CJDLogger::GetLogger().info("Loading Merge");

    
}