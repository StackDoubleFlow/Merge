#pragma once

#include <filesystem>
#include <string>

struct MModInfo {
    std::string assemblyName;
    std::string metadataFilename;
    std::string codeFilename;
};

struct RawMod {
    MModInfo modInfo;
    void *metadata;
    void *codeHandle;
};

class ModReader {
public:
    static std::vector<RawMod> ReadAllMods();

private:
    static std::string_view GetModsDirectory();
    static void *ReadFile(std::filesystem::path path);
    static MModInfo ReadModInfo(std::filesystem::path path);
};
