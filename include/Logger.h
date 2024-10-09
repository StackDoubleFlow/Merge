#pragma once
#include "paper/shared/logger.hpp"

constexpr auto MLogger = Paper::ConstLoggerContext("Merge");

struct Il2CppGlobalMetadataHeader;

namespace MDebugLog {

void LogMetadataHeader(const Il2CppGlobalMetadataHeader *header);

} // namespace MDebugLog
