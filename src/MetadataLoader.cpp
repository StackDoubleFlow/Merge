#include "MetadataLoader.h"
#include "Logger.h"
#include "beatsaber-hook/shared/config/config-utils.hpp"

#include <fstream>

namespace fs = std::filesystem;

std::string_view MetadataLoader::GetModsDirectory() {
    static std::string dir = getDataDir(MLogger::modInfo);
    dir += "/Mods";
    return dir;
}

void *MetadataLoader::LoadMetadata(std::filesystem::path path) {
    std::ifstream file;
    file.open(path, std::ios::in | std::ios::binary | std::ios::ate);
    if (file.is_open()) {
        std::streampos size = file.tellg();
        char *data = new char[size];
        file.seekg(0, std::ios::beg);
        file.read(data, size);
        file.close();
        MLogger::GetLogger().info("Succesfully loaded metadata file at %s",
                                  path.c_str());
    } else {
        MLogger::GetLogger().error("Error reading metadata at %s: %s",
                                   path.c_str(), strerror(errno));
        SAFE_ABORT();
    }
}

std::vector<void *> MetadataLoader::LoadAllMetadatas() {
    std::vector<void *> metadatas;
    std::string_view path = GetModsDirectory();
    std::error_code ec;
    auto directory_iterator =
        fs::directory_iterator(path, fs::directory_options::none, ec);
    if (ec) {
        std::string pathStr(path);
        MLogger::GetLogger().error("Error reading metadata directory at %s: %s",
                                   pathStr.c_str(), ec.message().c_str());
        SAFE_ABORT();
    }
    for (const auto &entry : directory_iterator) {
        metadatas.push_back(LoadMetadata(entry.path()));
    }
}
