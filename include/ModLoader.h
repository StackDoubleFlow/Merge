#pragma once

class ModLoader {
public:
    static void Initialize();
    // Loads all mods and returns the new metadata
    static void *LoadAllMods(void *baseMetadata);
};