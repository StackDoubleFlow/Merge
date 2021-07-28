#include "ModReader.h"
#include "Logger.h"
#include "beatsaber-hook/shared/config/config-utils.hpp"

#include <fstream>

namespace fs = std::filesystem;

std::string_view ModReader::GetModsDirectory() {
    static std::string dir = getDataDir(MLogger::modInfo);
    dir += "/Mods";
    return dir;
}

void *ModReader::ReadFile(std::filesystem::path path) {
    std::ifstream file;
    file.open(path, std::ios::in | std::ios::binary | std::ios::ate);
    if (file.is_open()) {
        std::streampos size = file.tellg();
        char *data = new char[size];
        file.seekg(0, std::ios::beg);
        file.read(data, size);
        file.close();
        return data;
    } else {
        MLogger::GetLogger().error("Error reading file at %s: %s", path.c_str(),
                                   strerror(errno));
        SAFE_ABORT();
        // unreachable
        return nullptr;
    }
}

MModInfo ModReader::ReadModInfo(std::filesystem::path path) {
    std::ifstream file(path);
    if (file.is_open()) {
        std::string str((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
        rapidjson::Document doc;
        doc.Parse(str);
        std::string assemblyName = doc["assemblyName"].GetString();
        std::string metadataFilename = doc["metadataFilename"].GetString();
        std::string codeFilename = doc["codeFilename"].GetString();
        return MModInfo{assemblyName, metadataFilename, codeFilename};
    } else {
        MLogger::GetLogger().error("Error reading file at %s: %s", path.c_str(),
                                   strerror(errno));
        SAFE_ABORT();
        // unreachable
        return MModInfo();
    }
}

std::vector<RawMod> ModReader::ReadAllMods() {
    std::vector<RawMod> mods;
    std::string_view path = GetModsDirectory();
    std::error_code ec;
    auto directory_iterator =
        fs::directory_iterator(path, fs::directory_options::none, ec);
    if (ec) {
        std::string pathStr(path);
        MLogger::GetLogger().error("Error reading mod directory at %s: %s",
                                   pathStr.c_str(), ec.message().c_str());
        SAFE_ABORT();
    }
    for (const auto &entry : directory_iterator) {
        fs::path path = entry.path();
        MModInfo modInfo = ReadModInfo(path / "mergeMod.json");
        void *metadata = ReadFile(path / modInfo.metadataFilename);
        void *code = ReadFile(path / modInfo.codeFilename);
        mods.push_back({modInfo, metadata, code});
        MLogger::GetLogger().info("Succesfully read mod %s",
                                  entry.path().c_str());
    }
    return mods;
}
