#pragma once

#include <filesystem>

class ModReader {
public:
    static void *ReadBaseMetadata();

private:
    static void *ReadFile(std::filesystem::path path);
};
