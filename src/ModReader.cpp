#include "ModReader.h"
#include "Logger.h"
#include "beatsaber-hook/shared/utils/utils-functions.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

namespace fs = std::filesystem;

void *ModReader::ReadBaseMetadata() {
    fs::path path = "/sdcard/Android/data";
    path = path / modloader::get_application_id();
    path = path / "files/il2cpp/Metadata/global-metadata.dat";
    return ReadFile(path);
}

void *ModReader::ReadFile(std::filesystem::path path) {
    int fd = open(path.c_str(), O_RDONLY);
    if (fd == -1) {
        MLogger.error("Error reading file at %s: %s", path.c_str(),
                      strerror(errno));
        SAFE_ABORT();
    }

    struct stat st;
    fstat(fd, &st);
    size_t size = st.st_size;

    void *mapped = mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0);
    return mapped;
}
