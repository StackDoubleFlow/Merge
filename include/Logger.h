#pragma once
#include "beatsaber-hook/shared/utils/logging.hpp"

struct Il2CppGlobalMetadataHeader;
class MLogger {
public:
    static inline ModInfo modInfo = ModInfo();
    static Logger &GetLogger() {
        static auto logger = new Logger(modInfo, LoggerOptions(false, true));
        return *logger;
    }
    static void LogMetadataHeader(const Il2CppGlobalMetadataHeader *header);
};