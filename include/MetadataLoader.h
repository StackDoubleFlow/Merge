#pragma once

#include <string>
#include <filesystem>

class MetadataLoader {
public:
    static void *LoadMetadata(std::filesystem::path path);
    static std::vector<void *> LoadAllMetadatas();

private:
    static std::string_view GetModsDirectory();
};