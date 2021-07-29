#include "MetadataBuilder.h"
#include "beatsaber-hook/shared/utils/utils.h"
#include "Logger.h"

template<typename T>
static T MetadataOffset(const void* metadata, size_t sectionOffset, size_t itemIndex) {
    return reinterpret_cast<T>(reinterpret_cast<const uint8_t*>(metadata) + sectionOffset) + itemIndex;
}

#define APPEND_BASE_FUCKED(name, countName) \
    for (size_t i = 0; i < header->countName / sizeof(decltype(name)::value_type); i++) { \
        decltype(name)::value_type item = *MetadataOffset<const decltype(name)::value_type*>(baseMetadata, header->name##Offset, i); \
        name.push_back(item); \
    }

#define APPEND_BASE(name) APPEND_BASE_FUCKED(name, name##Count)

MetadataBuilder::MetadataBuilder(const void *baseMetadata) {
    auto *header = static_cast<const Il2CppGlobalMetadataHeader *>(baseMetadata);
    CRASH_UNLESS(header->sanity == 0xFAB11BAF);
    CRASH_UNLESS(header->version == 24);

    APPEND_BASE(stringLiteral)
    APPEND_BASE(stringLiteralData)
    APPEND_BASE(string)
    APPEND_BASE(events)
    APPEND_BASE(properties)
    APPEND_BASE(methods)
    APPEND_BASE(parameterDefaultValues)
    APPEND_BASE(fieldDefaultValues)
    APPEND_BASE(fieldAndParameterDefaultValueData)
    APPEND_BASE(fieldMarshaledSizes)
    APPEND_BASE(parameters)
    APPEND_BASE(fields)
    APPEND_BASE(genericParameters)
    APPEND_BASE(genericParameterConstraints)
    APPEND_BASE(genericContainers)
    APPEND_BASE(nestedTypes)
    APPEND_BASE(interfaces)
    APPEND_BASE(vtableMethods)
    APPEND_BASE(interfaceOffsets)
    APPEND_BASE(typeDefinitions)
    APPEND_BASE(images)
    APPEND_BASE(assemblies)
    APPEND_BASE(metadataUsageLists)
    APPEND_BASE(metadataUsagePairs)
    APPEND_BASE(fieldRefs)
    APPEND_BASE(referencedAssemblies)
    APPEND_BASE(attributesInfo)
    APPEND_BASE(attributeTypes)
    APPEND_BASE(unresolvedVirtualCallParameterTypes)
    APPEND_BASE(unresolvedVirtualCallParameterRanges)
    APPEND_BASE_FUCKED(windowsRuntimeTypeNames, windowsRuntimeTypeNamesSize)
    APPEND_BASE(exportedTypeDefinitions)
}

void MetadataBuilder::AppendMetadata(const void *metadata, std::string_view assemblyName) {
    auto logger = MLogger::GetLogger().WithContext("MetadataBuilder::AppendMetadata");
    logger.debug("Appending metadata to builder from %p with assembly %s", metadata, assemblyName.data());
    auto *header = static_cast<const Il2CppGlobalMetadataHeader *>(metadata);
    CRASH_UNLESS(header->sanity == 0xFAB11BAF);
    CRASH_UNLESS(header->version == 24);

    for (size_t i = 0; i < header->assembliesCount / sizeof(Il2CppAssemblyDefinition); i++) {
        Il2CppAssemblyDefinition assembly = *MetadataOffset<const Il2CppAssemblyDefinition *>(metadata, header->assembliesOffset, i);
        const char *aname = MetadataOffset<const char *>(metadata, header->stringOffset, assembly.aname.nameIndex);
        if (assemblyName != aname) continue;
        logger.debug("Merging metadata from assembly %s", aname);

        const char *publicKey = MetadataOffset<const char *>(metadata, header->stringOffset, assembly.aname.publicKeyIndex);
        const char *culture = MetadataOffset<const char *>(metadata, header->stringOffset, assembly.aname.cultureIndex);
        assembly.aname.nameIndex = AppendString(aname);
        assembly.aname.publicKeyIndex = AppendString(publicKey);
        assembly.aname.cultureIndex = AppendString(culture);
        ImageIndex imageIndex = assembly.imageIndex;
        assembly.imageIndex = images.size();
        AssemblyIndex assemblyIndex = assemblies.size();
        assemblies.push_back(assembly);

        Il2CppImageDefinition image = *MetadataOffset<const Il2CppImageDefinition *>(metadata, header->imagesOffset, imageIndex);
        const char *imageName = MetadataOffset<const char *>(metadata, header->stringOffset, image.nameIndex);
        logger.debug("Adding image %s", imageName);
        image.nameIndex = AppendString(imageName);
        image.assemblyIndex = assemblyIndex;
        TypeDefinitionIndex typeStart = typeDefinitions.size();

        for (size_t i = 0; i < image.typeCount; i++) {
            Il2CppTypeDefinition type = *MetadataOffset<const Il2CppTypeDefinition *>(metadata, header->typeDefinitionsOffset, i + image.typeStart);
            const char *name = MetadataOffset<const char *>(metadata, header->stringOffset, type.nameIndex);
            const char *namespaze = MetadataOffset<const char *>(metadata, header->stringOffset, type.namespaceIndex);
            logger.debug("Adding type %s.%s", namespaze, name);
            type.nameIndex = AppendString(name);
            type.namespaceIndex = AppendString(namespaze);

            MethodIndex methodStart = typeDefinitions.size();
            for (size_t i = 0; i < type.method_count; i++) {
                Il2CppMethodDefinition method = *MetadataOffset<const Il2CppMethodDefinition *>(metadata, header->methodsOffset, i + type.methodStart);
                const char *name = MetadataOffset<const char *>(metadata, header->stringOffset, method.nameIndex);
                logger.debug("Adding method %s", name);
                method.nameIndex = AppendString(name);
                methods.push_back(method);
            }
            type.methodStart = methodStart;

            typeDefinitions.push_back(type);
        }
        image.typeStart = typeStart;

        images.push_back(image);
        return;
    }
}

StringIndex MetadataBuilder::AppendString(const char *str) {
    StringIndex idx = string.size();
    size_t len = strlen(str) + 1;
    for (size_t i = 0; i < len; i++) {
        string.push_back(str[i]);
    }
    return idx;
}

#define BUILD_METADATA_FUCKED(name, countName) \
    newHeader->name##Offset = i; \
    newHeader->countName = name.size() * sizeof(decltype(name)::value_type); \
    for (auto &item : name) { \
        *(reinterpret_cast<decltype(name)::value_type *>(reinterpret_cast<uint8_t *>(metadata) + i)) = item; \
        i += sizeof(decltype(name)::value_type); \
    }

#define BUILD_METADATA(name) BUILD_METADATA_FUCKED(name, name##Count)

void *MetadataBuilder::Finish() {
    // TODO: Calculate metadata size
    // but for right now I'll just do 20mb
    void *metadata = new char[20000000];
    Il2CppGlobalMetadataHeader *newHeader = static_cast<Il2CppGlobalMetadataHeader *>(metadata);
    newHeader->sanity = 0xFAB11BAF;
    newHeader->version = 24;

    int32_t i = sizeof(Il2CppGlobalMetadataHeader);
    BUILD_METADATA(stringLiteral)
    BUILD_METADATA(stringLiteralData)
    BUILD_METADATA(string)
    BUILD_METADATA(events)
    BUILD_METADATA(properties)
    BUILD_METADATA(methods)
    BUILD_METADATA(parameterDefaultValues)
    BUILD_METADATA(fieldDefaultValues)
    BUILD_METADATA(fieldAndParameterDefaultValueData)
    BUILD_METADATA(fieldMarshaledSizes)
    BUILD_METADATA(parameters)
    BUILD_METADATA(fields)
    BUILD_METADATA(genericParameters)
    BUILD_METADATA(genericParameterConstraints)
    BUILD_METADATA(genericContainers)
    BUILD_METADATA(nestedTypes)
    BUILD_METADATA(interfaces)
    BUILD_METADATA(vtableMethods)
    BUILD_METADATA(interfaceOffsets)
    BUILD_METADATA(typeDefinitions)
    BUILD_METADATA(images)
    BUILD_METADATA(assemblies)
    BUILD_METADATA(metadataUsageLists)
    BUILD_METADATA(metadataUsagePairs)
    BUILD_METADATA(fieldRefs)
    BUILD_METADATA(referencedAssemblies)
    BUILD_METADATA(attributesInfo)
    BUILD_METADATA(attributeTypes)
    BUILD_METADATA(unresolvedVirtualCallParameterTypes)
    BUILD_METADATA(unresolvedVirtualCallParameterRanges)
    BUILD_METADATA_FUCKED(windowsRuntimeTypeNames, windowsRuntimeTypeNamesSize)
    BUILD_METADATA(exportedTypeDefinitions)
    
    MLogger::GetLogger().debug("Built new metadata at %p with size %i", metadata, i);

    return metadata;
}
