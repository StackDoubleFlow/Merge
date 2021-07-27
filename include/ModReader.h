#pragma once

#include <string>
#include <filesystem>

struct RawMod {
    void *metadata;
    void *code;
};

class ModReader {
public:
    static std::vector<RawMod> ReadAllMods();

private:
    static std::string_view GetModsDirectory();
    static void *ReadFile(std::filesystem::path path);
};