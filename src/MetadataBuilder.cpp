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

#define APPEND_BASE(name) APPEND_BASE_FUCKED(name, name##Size)

void MetadataBuilder::Initialize(const void *baseMetadata) {
    auto *header = static_cast<const Il2CppGlobalMetadataHeader *>(baseMetadata);
    CRASH_UNLESS(header->sanity == 0xFAB11BAF);
    CRASH_UNLESS(header->version == 31);

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
    APPEND_BASE(fieldRefs)
    APPEND_BASE(referencedAssemblies)
    APPEND_BASE(attributeData)
    APPEND_BASE(attributeDataRange);
    APPEND_BASE(unresolvedIndirectCallParameterTypes)
    APPEND_BASE(unresolvedIndirectCallParameterRanges)
    APPEND_BASE(windowsRuntimeTypeNames);
    APPEND_BASE(windowsRuntimeStrings);
    APPEND_BASE(exportedTypeDefinitions)

    for (size_t i = 0; i < typeDefinitions.size(); i++) {
        Il2CppTypeDefinition &type = typeDefinitions[i];
        const char *name = MetadataOffset<const char *>(baseMetadata, header->stringOffset, type.nameIndex);
        const char *namespaze = MetadataOffset<const char *>(baseMetadata, header->stringOffset, type.namespaceIndex);
        typeNameMap[std::make_pair(namespaze, name)] = i;
    }
}

void MetadataBuilder::AppendMetadata(const void *metadata, std::string_view assemblyName, int typeOffset) {
    auto logger = Paper::ConstLoggerContext("Merge (MetadataBuilder::AppendMetadata)");
    logger.debug("Appending metadata to builder from %p with assembly %s", metadata, assemblyName.data());
    auto *header = static_cast<const Il2CppGlobalMetadataHeader *>(metadata);
    CRASH_UNLESS(header->sanity == 0xFAB11BAF);
    CRASH_UNLESS(header->version == 24);

    // TODO: maybe use these? idk if it's necessary 
    std::unordered_map<TypeIndex, TypeIndex> typeDefinitionRedirects;

    for (size_t i = 0; i < header->assembliesSize / sizeof(Il2CppAssemblyDefinition); i++) {
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
            typeNameMap[std::make_pair(namespaze, name)] = typeDefinitions.size();
            logger.debug("Adding type %s.%s, interfaces count: %hi, vtable size: %hi", namespaze, name, type.interfaces_count, type.vtable_count);
            type.nameIndex = AppendString(name);
            type.namespaceIndex = AppendString(namespaze);

            MethodIndex methodStart = methods.size();
            for (size_t i = 0; i < type.method_count; i++) {
                Il2CppMethodDefinition method = *MetadataOffset<const Il2CppMethodDefinition *>(metadata, header->methodsOffset, i + type.methodStart);
                const char *name = MetadataOffset<const char *>(metadata, header->stringOffset, method.nameIndex);
                logger.debug("Adding method %s, RID: %i, slot: %hi", name, method.token & 0x00FFFFFF, method.slot);
                method.nameIndex = AppendString(name);
                method.declaringType += typeOffset;
                method.returnType += typeOffset;
                ParameterIndex parameterStart = parameters.size();
                for (size_t i = 0; i < method.parameterCount; i++) {
                    Il2CppParameterDefinition param = *MetadataOffset<const Il2CppParameterDefinition *>(metadata, header->parametersOffset, i + method.parameterStart);
                    const char *name = MetadataOffset<const char *>(metadata, header->stringOffset, param.nameIndex);
                    param.nameIndex = AppendString(name);
                    param.typeIndex += typeOffset;
                    parameters.push_back(param);
                }
                method.parameterStart = parameterStart;
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

TypeDefinitionIndex MetadataBuilder::RedirectTypeDefinition(std::unordered_map<TypeDefinitionIndex, TypeDefinitionIndex> &typeRedirects,
                                                            TypeDefinitionIndex modType, ImageIndex imageIndex, const void *metadata) {
    auto *header = static_cast<const Il2CppGlobalMetadataHeader *>(metadata);

    const Il2CppImageDefinition *image = MetadataOffset<const Il2CppImageDefinition *>(metadata, header->imagesOffset, imageIndex);
    if (modType >= image->typeStart && modType < image->typeStart + image->typeCount) {
        // TODO: return the new type index
        return modType;
    }
    
    auto itr = typeRedirects.find(modType);
    if (itr != typeRedirects.end()) {
        return itr->second;
    }

    // Look for the mod image the type is in
    for (ImageIndex imageIndex = 0; imageIndex < header->imagesSize / sizeof(Il2CppImageDefinition); imageIndex++) {
        const Il2CppImageDefinition *modImage = MetadataOffset<const Il2CppImageDefinition *>(metadata, header->imagesOffset, imageIndex);
        const char *modImageName = MetadataOffset<const char *>(metadata, header->stringOffset, modImage->nameIndex);
        MLogger.info("Looking for %i in range %i..%i", modType, modImage->typeStart, modImage->typeStart + modImage->typeCount);
        if (modType >= modImage->typeStart && modType < modImage->typeStart + modImage->typeCount) {
            // Find our equivalent image
            for (Il2CppImageDefinition &image : images) {
                const char *imageName = &string[image.nameIndex];
                if (strcmp(modImageName, imageName) == 0) {
                    // This will only work if the offset in the base image is the same as the mod image
                    int typeOffset = modType - modImage->typeStart;
                    TypeIndex baseType = image.typeStart + modType;
                    typeRedirects[modType] = baseType;
                    return baseType;
                }
            }
            MLogger.error("Could not find equavalent image for %s", modImageName);
            SAFE_ABORT();
        }
    }
    MLogger.error("Could not mod image containing mod type %i", modType);
    SAFE_ABORT();
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

#define BUILD_METADATA(name) BUILD_METADATA_FUCKED(name, name##Size)

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
    BUILD_METADATA(fieldRefs)
    BUILD_METADATA(referencedAssemblies)
    BUILD_METADATA(attributeData)
    BUILD_METADATA(attributeDataRange)
    BUILD_METADATA(unresolvedIndirectCallParameterTypes)
    BUILD_METADATA(unresolvedIndirectCallParameterRanges)
    BUILD_METADATA(windowsRuntimeTypeNames)
    BUILD_METADATA(windowsRuntimeStrings)
    BUILD_METADATA(exportedTypeDefinitions)
    
    MLogger.debug("Built new metadata at %p with size %i", metadata, i);

    return metadata;
}

std::optional<TypeDefinitionIndex> MetadataBuilder::FindTypeDefinition(const char *namespaze, const char *name) {
    auto itr = typeNameMap.find(std::make_pair(namespaze, name));
    if (itr != typeNameMap.end()) {
        return itr->second;
    }
    // for (auto &[pair, idx] : typeNameMap) {
    //     auto &[namespaze, name] = pair;
    //     MLogger.info("%s.%s", namespaze.c_str(), name.c_str());
    // }
    return std::nullopt;
}